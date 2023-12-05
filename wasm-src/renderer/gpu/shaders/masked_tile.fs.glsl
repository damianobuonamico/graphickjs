R"(

  precision highp float;
  precision highp sampler2D;
  precision lowp usampler2D;

  uniform usampler2D uMasksTexture;
  uniform sampler2D uCoverTableTexture;
  uniform mediump int uMasksTextureSize;
  uniform mediump int uCoverTableTextureSize;
  uniform lowp int uTileSize;

  flat in vec2 vSegmentsCoords;
  flat in vec2 vCoverTableCoords;
  in vec4 vColor;
  in vec2 vCoords;

  out vec4 oFragColor;

  #define FRACBITS 8

  vec2 step_coords(vec2 coords, int delta, float one_over_size) {
    coords.x += float(delta) * one_over_size;

    while (coords.x >= 1.0) {
      coords.x -= 1.0;
      coords.y += one_over_size;
    }

    return coords;
  }

  float x_intersect(float one_over_m, float q, float y) {
    return (y - q) * one_over_m;
  }

  float y_intersect(float m, float q, float x) {
    return m * x + q;
  }

  void main() {
    float one_over_size = 1.0 / float(uMasksTextureSize);
    float one_over_cover_table_size = 1.0 / float(uCoverTableTextureSize);
    float tile_size_over_255 = float(uTileSize) / 255.0;

    vec2 coords = vSegmentsCoords;
    vec2 cover_table = vCoverTableCoords;

    uvec4 metadata = texture(uMasksTexture, coords);
    int n = int(metadata.r | (metadata.g << 8) | (metadata.b << 16) | (metadata.a << 24));

    float y0 = floor(vCoords.y);
    float y1 = y0 + 1.0;
    float x0 = floor(vCoords.x);
    float x1 = x0 + 1.0;

    float alpha = texture(uCoverTableTexture, cover_table + vec2(y0 * one_over_cover_table_size, 0.0)).r;

    for (int i = 0; i < n; i++) {
      vec2 p0_coords = step_coords(coords, i * 2 + 1, one_over_size);
      vec2 p3_coords = step_coords(coords, i * 2 + 2, one_over_size);
      
      uvec4 p0_raw = texture(uMasksTexture, p0_coords);
      uvec4 p3_raw = texture(uMasksTexture, p3_coords);

      vec2 p0 = vec2(float((p0_raw.r << FRACBITS) + p0_raw.g) / float(1 << FRACBITS), float((p0_raw.b << FRACBITS) + p0_raw.a) / float(1 << FRACBITS));
      vec2 p3 = vec2(float((p3_raw.r << FRACBITS) + p3_raw.g) / float(1 << FRACBITS), float((p3_raw.b << FRACBITS) + p3_raw.a) / float(1 << FRACBITS));

      if (min(p0.y, p3.y) >= y1 || max(p0.y, p3.y) <= y0) {
        // Segment is outside the scanline
        continue;
      }

      float area = 0.0;
      float cover = 0.0;

      // Check if segment is almost vertical
      if (abs(p0.x - p3.x) < 0.1) {
        // Treating segment as vertical, horizontal segments are discarded in CPU preprocessing
          
        if (p0.x < x1) {
          alpha += (clamp(p3.y, y0, y1) - clamp(p0.y, y0, y1)) * (p0.x < x0 ? 1.0 : x1 - p0.x);
        }

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
    float opacity = /*vColor.a * */min(abs(alpha), 1.0);
    // Even-odd winding rule
    // float opacity = /*vColor.a * */abs(alpha - 2.0 * round(0.5 * alpha));
    if (opacity < 0.01) opacity = 0.0;

    oFragColor = vec4(vColor.rgb, opacity);
  }

)"
