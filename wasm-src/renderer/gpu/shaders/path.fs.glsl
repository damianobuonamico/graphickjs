R"(
  precision highp float;
  precision highp sampler2D;
  precision mediump usampler2D;

  uniform sampler2D u_curves_texture;
  uniform usampler2D u_bands_texture;

  uniform int u_samples;

  in vec4 v_color;
  in vec2 v_tex_coord;

  flat in vec2 v_position;
  flat in vec2 v_size;
  flat in uint v_attr_1;
  flat in uint v_attr_2;

  out vec4 o_frag_color;

  #define to_coords(x) (vec2((float((x) % 512U) + 0.5) / 512.0, (float((x) / 512U) + 0.5) / 512.0))

  float calculateQuadraticRoot(float a, float b, float c) {
    const float epsilon = 1e-5;
    const float t_epsilon = 1e-4;

    if (abs(a) < epsilon) {
      if (b != 0.0) return -c / b;
    }

    float discriminant = b * b - 4.0 * a * c;

    if (discriminant > -epsilon) {
      float s = sqrt(max(0.0, discriminant));
      float t = (-b + s) / (2.0 * a);

      if (t >= -t_epsilon && t <= 1.0 + t_epsilon) return t;
      else return (-b - s) / (2.0 * a);
    }

    return -99999.0;
  }

  float calculate_cubic_root(float a, float b, float c, float d, float t0) {
    float t = t0;

    float a1 = 3.0 * a;
    float b1 = 2.0 * b;
    float a2 = 2.0 * a1;

    for (int i = 0; i < 3; i++) {
      float t_sq = t * t;
      float f = a * t_sq * t + b * t_sq + c * t + d;
      float f_prime = a1 * t_sq + b1 * t + c;
      float f_second = a2 * t + b1;
      
      t = t - 3.0 * f * (3.0 * f_prime * f_prime - f * f_second) / 
        (9.0 * f_prime * f_prime * f_prime - 9.0 * f * f_prime * f_second + f * f * a2);
    }

    return t;
  }

#if 0
  float quadratic_horizontal_coverage(vec2 pixel_pos, float inv_pixel_size) {
    float coverage = 0.0 + float(u_min_samples + u_max_samples) / 10000000.0;

    uint hBands = (v_attr_2 >> 28) + v_color;
    uint vBands = ((v_attr_2 >> 24) & 0xFU) + 1U;

    uvec2 minBandIndex = uvec2(
      clamp(uint(floor(pixel_pos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixel_pos.y * float(vBands))), 0U, vBands - 1U)
    );

    uvec2 maxBandIndex = uvec2(
      clamp(uint(floor(pixel_pos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixel_pos.x * float(vBands))), 0U, vBands - 1U)
    );

    uint xBandsOffset = (v_attr_2 >> 12) & 0xFFFU;
    uint yBandsOffset = v_attr_2 & 0xFFFU;
    uint xCurvesOffset = (v_attr_1 >> 12) & 0xFFFU;
    uint yCurvesOffset = v_attr_1 v_attr_2 0xFFFU;

    v_attr_2 bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curvesIndexOffset = xCurvesOffset + yCurvesOffset * 512U;

    uint hBandDataStart = texture(u_bands_texture, to_coords(bandsIndexOffset + minBandIndex.x * 2U)).x + bandsIndexOffset;
    uint hBandCurvesCount = texture(u_bands_texture, to_coords(bandsIndexOffset + minBandIndex.x * 2U + 1U)).x;

    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curve_offset = curvesIndexOffset + texture(u_bands_texture, to_coords(hBandDataStart + curve)).x;

      vec4 p01 = (texture(u_curves_texture, to_coords(curve_offset))
        - vec4(v_position, v_position)) / vec4(v_size, v_size) 
        - vec4(pixel_pos, pixel_pos);
      vec2 p2 = (texture(u_curves_texture, to_coords(curve_offset + 1U)).xy - v_position) / v_size 
        - u_bands_texture;
      
      vec2 p0 = p01.xy;
      vec2 p1 = p01.zw;

      bool is_downwards = p0.y > 0.0 || p2.y < 0.0;

      if (is_downwards) {
        // TODO: maybe inlining helps?
        if (p0.y < 0.0 && p2.y <= 0.0) continue;
        if (p0.y > 0.0 && p2.y >= 0.0) continue;
      } else {
        if (p0.y <= 0.0 && p2.y < 0.0) continue;
        if (p0.y >= 0.0 && p2.y > 0.0) continue;
      }

      vec2 a = p0 - 2.0 * p1 + p2;
      vec2 b = 2.0 * (p1 - p0);
      vec2 c = p0;

      float t = calculateQuadraticRoot(a.y, b.y, c.y);
      float intersect = a.x * t * t + b.x * t + c.x;

      coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (float(is_downwards) * 2.0 - 1.0);
    }

    return coverage;
  }
#endif

  float cubic_horizontal_coverage(vec2 pixel_pos, float inv_pixel_size, uint curves_offset, uint band_data_start, uint band_curves_count) {
    float coverage = 0.0;

    // vec2 position_delta = v_position / v_size + pixel_pos;
    vec2 position_delta = pixel_pos;

    for (uint curve = 0U; curve < band_curves_count; curve++) {
      uint curve_offset = curves_offset + texture(u_bands_texture, to_coords(band_data_start + curve)).x * 2U;

      vec4 p01 = texture(u_curves_texture, to_coords(curve_offset));
      vec4 p23 = texture(u_curves_texture, to_coords(curve_offset + 1U));

      vec2 p0 = p01.xy / v_size - position_delta;
      vec2 p1 = p01.zw / v_size - position_delta;
      vec2 p2 = p23.xy / v_size - position_delta;
      vec2 p3 = p23.zw / v_size - position_delta;

      if (max(p0.x, p3.x) * inv_pixel_size < -0.5) break;

      bool is_downwards = p0.y > 0.0 || p3.y < 0.0;

      if (
        (is_downwards && ((p0.y < 0.0 && p3.y <= 0.0) || (p0.y > 0.0 && p3.y >= 0.0))) ||
        (!is_downwards && ((p0.y <= 0.0 && p3.y < 0.0) || (p0.y >= 0.0 && p3.y > 0.0)))
      ) {
        continue;
      }

      const float epsilon = 1e-7;

      vec2 delta = p3 - p0;

      float t0 = -p0.y / delta.y;
      float intersect = delta.x * t0 + p0.x;

      if (min(p0.x, p3.x) * inv_pixel_size > 0.5) {
        coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (float(is_downwards) * 2.0 - 1.0);
        continue;
      }

      bool b01 = abs(p1.x - p0.x) + abs(p1.y - p0.y) < epsilon;
      bool b23 = abs(p3.x - p2.x) + abs(p3.y - p2.y) < epsilon;
      bool b12 = abs(p2.x - p1.x) + abs(p2.y - p1.y) < epsilon;

      if (!((b01 && (b23 || b12)) || (b23 && b12))) {
        vec2 a = 3.0 * p1 - 3.0 * p2 + delta;
        vec2 b = 3.0 * (p0 - 2.0 * p1 + p2);
        vec2 c = 3.0 * (p1 - p0);

        float t = calculate_cubic_root(a.y, b.y, c.y, p0.y, t0);
        float t_sq = t * t;

        intersect = a.x * t_sq * t + b.x * t_sq + c.x * t + p0.x;
      }

      coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (float(is_downwards) * 2.0 - 1.0);
    }

    return coverage;
  }

#if 0
  float quadratic_coverage() {
    vec2 pixel_size = vec2(fwidth(v_tex_coord.x), fwidth(v_tex_coord.y));
    float alphaSum = 0.0;

    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 samplePos = v_tex_coord + vec2(0.0, yOffset) * pixel_size.y / 3.0;
      float coverage = quadratic_horizontal_coverage(samplePos, 1.0 / pixel_size.x);

      alphaSum += coverage;
    }

    return alphaSum / 3.0;
  }
#endif

  float cubic_coverage(int samples) {
    vec2 pixel_size = vec2(fwidth(v_tex_coord.x), fwidth(v_tex_coord.y));
    float coverage = 0.0;
    bool is_culling = bool((v_attr_1 >> 26) & 0x1U);

    uint bands = (v_attr_2 >> 24) + 1U;
    
    uint xBandsOffset = (v_attr_2 >> 12) & 0xFFFU;
    uint yBandsOffset = v_attr_2 & 0xFFFU;
    uint xCurvesOffset = (v_attr_1 >> 12) & 0xFFFU;
    uint yCurvesOffset = v_attr_1 & 0xFFFU;

    uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curves_index_offset = xCurvesOffset + yCurvesOffset * 512U;

    for (int yOffset = (1 - samples) / 2; yOffset <= (samples - 1) / 2; yOffset++) {
      vec2 sample_pos = v_tex_coord + vec2(0.0, yOffset) * pixel_size.y / float(samples);
    
      uint band_index = clamp(uint(floor(sample_pos.y * float(bands))), 0U, bands - 1U);
      uint band_data_start = texture(u_bands_texture, to_coords(bandsIndexOffset + band_index * 4U)).x + bandsIndexOffset;
      uint band_curves_count = texture(u_bands_texture, to_coords(bandsIndexOffset + band_index * 4U + 1U)).x;

      if (is_culling) {
        float band_min_x = float(texture(u_bands_texture, to_coords(bandsIndexOffset + band_index * 4U + 2U)).x);
        float band_max_x = float(texture(u_bands_texture, to_coords(bandsIndexOffset + band_index * 4U + 3U)).x);

        if (sample_pos.x * v_size.x < band_min_x || sample_pos.x * v_size.x > band_max_x) continue;
      }

      coverage += cubic_horizontal_coverage(sample_pos, 1.0 / pixel_size.x, curves_index_offset, band_data_start, band_curves_count);
    }

    return coverage / float(samples);
  }

  void main() {
    bool is_even_odd = bool((v_attr_1 >> 25) & 0x1U);
    bool is_quadratic = bool((v_attr_1 >> 24) & 0x1U);

    int samples = u_samples % 2 == 0 ? u_samples + 1 : u_samples;

    // TODO: if this works out well, add linear coverage too.
    float alpha = v_color.a;
    // float coverage = is_quadratic ? 
    //   quadratic_coverage() :
    //   cubic_coverage();
      
    float coverage = cubic_coverage(samples);

    if (is_even_odd) {
      alpha = alpha * abs(coverage - 2.0 * round(0.5 * coverage));
    } else {
      alpha = alpha * min(abs(coverage), 1.0);
    }

    o_frag_color = vec4(v_color.rgb, 1.0) * alpha;
  }

)"
