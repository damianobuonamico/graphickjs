R"(
  // BASE: 10-12-13ms

  precision highp float;
  precision highp sampler2D;
  precision mediump usampler2D;

  uniform sampler2D uCurvesTexture;
  uniform usampler2D uBandsTexture;

  uniform int uMinSamples;
  uniform int uMaxSamples;

  in vec4 vColor;
  in vec2 vTexCoord;

  flat in vec2 vPosition;
  flat in vec2 vSize;
  flat in uint vCurvesData;
  flat in uint vBandsData;

  out vec4 oFragColor;

  #define H_ONLY 1
  #define V_ONLY 0
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

    for (int i = 0; i < 3; i++) {
      float t_sq = t * t;
      float f = a * t_sq * t + b * t_sq + c * t + d;
      float f_prime = 3.0 * a * t_sq + 2.0 * b * t + c;
      float f_second = 6.0 * a * t + 2.0 * b;
      float f_third = 6.0 * a;
      
      t = t - 3.0 * f * (3.0 * f_prime * f_prime - f * f_second) / 
        (9.0 * f_prime * f_prime * f_prime - 9.0 * f * f_prime * f_second + f * f * f_third);
    }

    return t;
  }

  float quadratic_horizontal_coverage(vec2 pixel_pos, float inv_pixel_size) {
    float coverage = 0.0 + float(uMinSamples + uMaxSamples) / 10000000.0;

    uint hBands = (vBandsData >> 28) + 1U;
    uint vBands = ((vBandsData >> 24) & 0xFU) + 1U;

    uvec2 minBandIndex = uvec2(
      clamp(uint(floor(pixel_pos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixel_pos.y * float(vBands))), 0U, vBands - 1U)
    );

    uvec2 maxBandIndex = uvec2(
      clamp(uint(floor(pixel_pos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixel_pos.x * float(vBands))), 0U, vBands - 1U)
    );

    uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
    uint yBandsOffset = vBandsData & 0xFFFU;
    uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
    uint yCurvesOffset = vCurvesData & 0xFFFU;

    uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curvesIndexOffset = xCurvesOffset + yCurvesOffset * 512U;

    uint hBandDataStart = texture(uBandsTexture, to_coords(bandsIndexOffset + minBandIndex.x * 2U)).x + bandsIndexOffset;
    uint hBandCurvesCount = texture(uBandsTexture, to_coords(bandsIndexOffset + minBandIndex.x * 2U + 1U)).x;

    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curve_offset = curvesIndexOffset + texture(uBandsTexture, to_coords(hBandDataStart + curve)).x;

      vec4 p01 = (texture(uCurvesTexture, to_coords(curve_offset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize) 
        - vec4(pixel_pos, pixel_pos);
      vec2 p2 = (texture(uCurvesTexture, to_coords(curve_offset + 1U)).xy - vPosition) / vSize 
        - pixel_pos;
      
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

  float cubic_horizontal_coverage(vec2 pixel_pos, float inv_pixel_size, uint curves_offset, uint band_data_start, uint band_curves_count) {
    float coverage = 0.0;

    vec2 position_delta = vPosition / vSize + pixel_pos;

    for (uint curve = 0U; curve < band_curves_count; curve++) {
      uint curve_offset = curves_offset + texture(uBandsTexture, to_coords(band_data_start + curve)).x * 2U;

      vec4 p01 = texture(uCurvesTexture, to_coords(curve_offset));
      vec4 p23 = texture(uCurvesTexture, to_coords(curve_offset + 1U));

      vec2 p0 = p01.xy / vSize - position_delta;
      vec2 p1 = p01.zw / vSize - position_delta;
      vec2 p2 = p23.xy / vSize - position_delta;
      vec2 p3 = p23.zw / vSize - position_delta;

      if (max(p0.x, p3.x) * inv_pixel_size < -0.5) break;

      bool is_downwards = p0.y > 0.0 || p3.y < 0.0;

      if (
        (is_downwards && ((p0.y < 0.0 && p3.y <= 0.0) || (p0.y > 0.0 && p3.y >= 0.0))) ||
        (!is_downwards && ((p0.y <= 0.0 && p3.y < 0.0) || (p0.y >= 0.0 && p3.y > 0.0)))
      ) {
        continue;
      }

      const float epsilon = 1e-7;

      bool b01 = abs(p1.x - p0.x) + abs(p1.y - p0.y) < epsilon;
      bool b23 = abs(p3.x - p2.x) + abs(p3.y - p2.y) < epsilon;
      bool b12 = abs(p2.x - p1.x) + abs(p2.y - p1.y) < epsilon;

      vec2 delta = p3 - p0;

      float intersect = 0.0;
      float t0 = -p0.y / delta.y;

      if ((b01 && (b23 || b12)) || (b23 && b12)) {
        intersect = delta.x * t0 + p0.x;
      } else {
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

  float cubic_vertical_coverage(vec2 pixel_pos, float inv_pixel_size, uint curves_offset, uint band_data_start, uint band_curves_count) {
    float coverage = 0.0;

    vec2 position_delta = vPosition / vSize + pixel_pos;

    for (uint curve = 0U; curve < band_curves_count; curve++) {
      uint curve_offset = curves_offset + texture(uBandsTexture, to_coords(band_data_start + curve)).x * 2U;

      vec4 p01 = texture(uCurvesTexture, to_coords(curve_offset));
      vec4 p23 = texture(uCurvesTexture, to_coords(curve_offset + 1U));

      vec2 p0 = p01.xy / vSize - position_delta;
      vec2 p1 = p01.zw / vSize - position_delta;
      vec2 p2 = p23.xy / vSize - position_delta;
      vec2 p3 = p23.zw / vSize - position_delta;

      if (max(p0.y, p3.y) * inv_pixel_size < -0.5) break;

      bool is_rightwards = p0.x > 0.0 || p3.x < 0.0;

      if (
        (is_rightwards && ((p0.x < 0.0 && p3.x <= 0.0) || (p0.x > 0.0 && p3.x >= 0.0))) ||
        (!is_rightwards && ((p0.x <= 0.0 && p3.x < 0.0) || (p0.x >= 0.0 && p3.x > 0.0)))
      ) {
        continue;
      }

      const float epsilon = 1e-7;

      bool b01 = abs(p1.y - p0.y) + abs(p1.x - p0.x) < epsilon;
      bool b23 = abs(p3.y - p2.y) + abs(p3.x - p2.x) < epsilon;
      bool b12 = abs(p2.y - p1.y) + abs(p2.x - p1.x) < epsilon;

      vec2 delta = p3 - p0;

      float intersect = 0.0;
      float t0 = -p0.x / delta.x;

      if ((b01 && (b23 || b12)) || (b23 && b12)) {
        intersect = delta.y * t0 + p0.y;
      } else {
        vec2 a = 3.0 * p1 - 3.0 * p2 + delta;
        vec2 b = 3.0 * (p0 - 2.0 * p1 + p2);
        vec2 c = 3.0 * (p1 - p0);

        float t = calculate_cubic_root(a.x, b.x, c.x, p0.x, t0);
        float t_sq = t * t;

        intersect = a.y * t_sq * t + b.y * t_sq + c.y * t + p0.y;
      }

      coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (1.0 - float(is_rightwards) * 2.0);
    }

    return coverage;
  }

  float quadratic_coverage() {
    vec2 pixel_size = vec2(fwidth(vTexCoord.x), fwidth(vTexCoord.y));
    float alphaSum = 0.0;

    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 samplePos = vTexCoord + vec2(0.0, yOffset) * pixel_size.y / 3.0;
      float coverage = quadratic_horizontal_coverage(samplePos, 1.0 / pixel_size.x);

      alphaSum += coverage;
    }

    return alphaSum / 3.0;
  }

  float cubic_coverage() {
    vec2 pixel_size = vec2(fwidth(vTexCoord.x), fwidth(vTexCoord.y));
    float coverage = 0.0;

    uvec2 bands = uvec2(
      (vBandsData >> 28) + 1U,
      ((vBandsData >> 24) & 0xFU) + 1U
    );
    
    uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
    uint yBandsOffset = vBandsData & 0xFFFU;
    uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
    uint yCurvesOffset = vCurvesData & 0xFFFU;

    uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curves_index_offset = xCurvesOffset + yCurvesOffset * 512U;

    float alphaSum = 0.0;
  
#if H_ONLY
    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 sample_pos = vTexCoord + vec2(0.0, yOffset) * pixel_size.y / 3.0;
    
      uint band_index = clamp(uint(floor(sample_pos.y * float(bands.x))), 0U, bands.x - 1U);
      uint band_data_start = texture(uBandsTexture, to_coords(bandsIndexOffset + band_index * 2U)).x + bandsIndexOffset;
      uint band_curves_count = texture(uBandsTexture, to_coords(bandsIndexOffset + band_index * 2U + 1U)).x;

      float cove = cubic_horizontal_coverage(sample_pos, 1.0 / pixel_size.x, curves_index_offset, band_data_start, band_curves_count);

      alphaSum += cove;
    }

    return alphaSum / 3.0;
#elif V_ONLY
    for (int xOffset = -1; xOffset <= 1; xOffset++) {
      vec2 sample_pos = vTexCoord + vec2(xOffset, 0.0) * pixel_size.x / 3.0;

      uint band_index = clamp(uint(floor(sample_pos.x * float(bands.y))), 0U, bands.y - 1U);
      uint band_data_start = texture(uBandsTexture, to_coords(bandsIndexOffset + (bands.x + band_index) * 2U)).x + bandsIndexOffset;
      uint band_curves_count = texture(uBandsTexture, to_coords(bandsIndexOffset + (bands.x + band_index) * 2U + 1U)).x;

      float cove = cubic_vertical_coverage(sample_pos, 1.0 / pixel_size.y, curves_index_offset, band_data_start, band_curves_count);

      alphaSum += cove;
    }

    return alphaSum / 3.0;
#else
    for (int yOffset = 0; yOffset <= 0; yOffset++) {
      vec2 sample_pos = vTexCoord + vec2(0.0, yOffset) * pixel_size.y / 3.0;

      uint band_index = clamp(uint(floor(sample_pos.y * float(bands.x))), 0U, bands.x - 1U);
      uint band_data_start = texture(uBandsTexture, to_coords(bandsIndexOffset + band_index * 2U)).x + bandsIndexOffset;
      uint band_curves_count = texture(uBandsTexture, to_coords(bandsIndexOffset + band_index * 2U + 1U)).x;

      float cove = cubic_horizontal_coverage(sample_pos, 1.0 / pixel_size.x, curves_index_offset, band_data_start, band_curves_count);

      alphaSum += cove;
    }
    
    for (int xOffset = -0; xOffset <= 0; xOffset++) {
      vec2 sample_pos = vTexCoord + vec2(xOffset, 0.0) * pixel_size.x / 3.0;

      uint band_index = clamp(uint(floor(sample_pos.x * float(bands.y))), 0U, bands.y - 1U);
      uint band_data_start = texture(uBandsTexture, to_coords(bandsIndexOffset + (bands.x + band_index) * 2U)).x + bandsIndexOffset;
      uint band_curves_count = texture(uBandsTexture, to_coords(bandsIndexOffset + (bands.x + band_index) * 2U + 1U)).x;

      float cove = cubic_vertical_coverage(sample_pos, 1.0 / pixel_size.y, curves_index_offset, band_data_start, band_curves_count);

      alphaSum += cove;
    }

    return alphaSum / 2.0;
#endif

    vec2 sample_pos = vTexCoord;

    uvec2 band_index = uvec2(
      clamp(uint(floor(sample_pos.y * float(bands.x))), 0U, bands.x - 1U),
      clamp(uint(floor(sample_pos.x * float(bands.y))), 0U, bands.y - 1U)
    );
    uvec2 band_data_start = uvec2(
      texture(uBandsTexture, to_coords(bandsIndexOffset + band_index.x * 2U)).x + bandsIndexOffset,
      texture(uBandsTexture, to_coords(bandsIndexOffset + (bands.x + band_index.y) * 2U)).x + bandsIndexOffset
    );
    uvec2 band_curves_count = uvec2(
      texture(uBandsTexture, to_coords(bandsIndexOffset + band_index.x * 2U + 1U)).x,
      texture(uBandsTexture, to_coords(bandsIndexOffset + (bands.x + band_index.y) * 2U + 1U)).x
    );

    coverage += cubic_horizontal_coverage(sample_pos - 0.2 * pixel_size.x, 1.0 / pixel_size.x, curves_index_offset, band_data_start.x, band_curves_count.x);
    coverage -= cubic_vertical_coverage(sample_pos + 0.2 * pixel_size.y, 1.0 / pixel_size.y, curves_index_offset, band_data_start.y, band_curves_count.y);

    return coverage * 0.5;
  }

  void main() {
    bool is_quadratic = bool(vCurvesData >> 28);
    bool is_even_odd = bool((vCurvesData >> 24) & 0xFU);

    // TODO: if this works out well, add linear coverage too.
    float alpha = vColor.a;
    float coverage = is_quadratic ? 
      quadratic_coverage() :
      cubic_coverage();
      
    if (is_even_odd) {
      alpha = alpha * sqrt(abs(coverage - 2.0 * round(0.5 * coverage)));
    } else {
#if H_ONLY || V_ONLY
      alpha = alpha * min(abs(coverage), 1.0);
#else
      alpha = alpha * sqrt(min(abs(coverage), 1.0));
#endif
    }

    oFragColor = vec4(vColor.rgb, 1.0) * alpha;
  }

)"
