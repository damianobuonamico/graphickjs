R"(

  precision highp float;
  precision highp sampler2D;

  uniform sampler2D uMasksTexture;
  uniform mediump int uMasksTextureSize;
  uniform lowp int uTileSize;

  flat in vec2 vSegmentsCoords;
  in vec4 vColor;
  in vec2 vCoords;

  out vec4 oFragColor;

  vec2 step_coords(vec2 coords, int delta, float one_over_size) {
    coords.x += float(delta) * one_over_size;

    while (coords.x >= 1.0) {
      coords.x -= 1.0;
      coords.y += one_over_size;
    }

    return coords;
  }

  float data_to_coverage(float data) {
    return 2.0 * data * 255.0 / 254.0 - 1.0;
  }

  float x_intersect(float one_over_m, float q, float y) {
    return (y - q) * one_over_m;
  }

  float y_intersect(float m, float q, float x) {
    return m * x + q;
  }

  void main() {
    float one_over_size = 1.0 / float(uMasksTextureSize);
    float tile_size_over_255 = float(uTileSize) / 255.0;

    vec2 coords = vSegmentsCoords;

    vec4 metadata = texture(uMasksTexture, coords) * 255.0;
    int n = int(uint(metadata.r) | (uint(metadata.g) << 8) | (uint(metadata.b) << 16) | (uint(metadata.a) << 24));

    // oFragColor = vec4(metadata.rgb, vColor.a);
    // return;

    float y0 = floor(vCoords.y);
    float y1 = y0 + 1.0;
    float x0 = floor(vCoords.x);
    float x1 = x0 + 1.0;

    vec4 coverage_block = texture(uMasksTexture, step_coords(coords, 1 + int(y0) / 4, one_over_size));
    coords = step_coords(coords, uTileSize / 4, one_over_size);

    float alpha = 0.0;

    int reminder = int(y0) % 4;
    if (reminder == 0) alpha += data_to_coverage(coverage_block.r);
    else if (reminder == 1) alpha += data_to_coverage(coverage_block.g);
    else if (reminder == 2) alpha += data_to_coverage(coverage_block.b);
    else if (reminder == 3) alpha += data_to_coverage(coverage_block.a);

    // oFragColor = vec4(vColor.rgb, abs(alpha - 2.0 * round(0.5 * alpha)));
    // return;

    for (int i = 1; i <= n; i++) {
      vec2 segment_cords = step_coords(coords, i, one_over_size);
      vec4 segment = texture(uMasksTexture, segment_cords) * float(uTileSize);

      vec2 p0 = segment.rg;
      vec2 p3 = segment.ba;

      if (min(p0.y, p3.y) >= y1 || max(p0.y, p3.y) <= y0) {
        // Segment is outside the scanline
        continue;
      }

      float area = 0.0;
      float cover = 0.0;

      // Check if segment is almost vertical
      if (abs(p0.x - p3.x) < 0.1) {
        // Treating segment as vertical, horizontal segments are discarded in CPU preprocessing
        if (p0.x > x1) {
          // Segment is on the right side of the pixel
          continue;
        }

        if (p0.x < x0) {
          // Segment is on the left side of the pixel
          cover += clamp(p3.y, y0, y1) - clamp(p0.y, y0, y1);
        } else {
          // Segment is inside the pixel
          area += (clamp(p3.y, y0, y1) - clamp(p0.y, y0, y1)) * (x1 - p0.x);
        }

        alpha += area + cover;

        continue;
      }

      float m = (p3.y - p0.y) / (p3.x - p0.x);
      float one_over_m = 1.0 / m;
      float q = p0.y - m * p0.x;

      // Clip segment's p0 to scanline
      if (p0.y < y0) {
        p0 = vec2(x_intersect(one_over_m, q, y0), y0);
      } else if (p0.y > y1) {
        p0 = vec2(x_intersect(one_over_m, q, y1), y1);
      }

      // Clip segment's p3 to scanline
      if (p3.y < y0) {
        p3 = vec2(x_intersect(one_over_m, q, y0), y0);
      } else if (p3.y > y1) {
        p3 = vec2(x_intersect(one_over_m, q, y1), y1);
      }

      if (min(p0.x, p3.x) >= x1) {
        // Segment is on the right side of the pixel
        continue;
      }

      // Clip segment to the right side of pixel
      if (p0.x > x1) {
        p0 = vec2(x1, y_intersect(m, q, x1));
      } else if (p3.x > x1) {
        p3 = vec2(x1, y_intersect(m, q, x1));
      }

      // Calculate area and cover values
      if (p0.x < x0) {
        if (p3.x < x0) {
          // Both points are on the left side of the pixel
          cover += p3.y - p0.y;
        } else {
          // p0 is on the left side of the pixel, p3 is on the right side, clipping
          vec2 p0_clip = vec2(x0, y_intersect(m, q, x0));
          area += 0.5 * (1.0 + x1 - p3.x) * (p3.y - p0_clip.y);
          cover += p0_clip.y - p0.y;
        }
      } else if (p3.x < x0) {
        // p3 is on the left side of the pixel, p0 is on the right side, clipping
        vec2 p3_clip = vec2(x0, y_intersect(m, q, x0));
        area += 0.5 * (1.0 + x1 - p0.x) * (p3_clip.y - p0.y);
        cover += p3.y - p3_clip.y;
      } else {
        // Both points are on the right side of the pixel
        area += 0.5 * (x1 - p0.x + x1 - p3.x) * (p3.y - p0.y);
      }

      alpha += area + cover;
    }

    // Non-zero winding rule
    // float opacity = vColor.a * min(abs(alpha), 1.0);
    // Even-odd winding rule
    float opacity = /*vColor.a * */abs(alpha - 2.0 * round(0.5 * alpha));
    if (opacity < 0.01) opacity = 0.0;

    oFragColor = vec4(vColor.rgb, opacity);

    // oFragColor = vec4(float(n) / 10.0, 0.0, 0.0, a);
  }

)"
