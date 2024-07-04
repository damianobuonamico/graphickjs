R"(
  // BASE: 4.01ms

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

  vec2 toCoords(uint x) {
    return vec2(float(x % 512U) + 0.5, float(x / 512U) + 0.5) / 512.0;
  }

  float calculateQuadraticRoot(float a, float b, float c) {
    const float epsilon = 1e-5;
    const float t_epsilon = 1e-4;

    if (abs(a) < epsilon) {
      if (b != 0) return -c / b;
    }

    float discriminant = b * b - 4 * a * c;

    if (discriminant > -epsilon) {
      float s = sqrt(max(0, discriminant));
      float t = (-b + s) / (2 * a);

      if (t >= -t_epsilon && t <= 1 + t_epsilon) return t;
      else return (-b - s) / (2 * a);
    }

    return -99999.0;
  }

  float calculate_cubic_root(float a, float b, float c, float d, float t0) {
    float t = t0;

    for (int i = 0; i < 3; i++) {
      float t_sq = t * t;
      float f = a * t_sq * t + b * t_sq + c * t + d;
      float f_prime = 3 * a * t_sq + 2 * b * t + c;
      float f_second = 6 * a * t + 2 * b;
      float f_third = 6 * a;
      
      t = t - 3 * f * (3 * f_prime * f_prime - f * f_second) / 
        (9 * f_prime * f_prime * f_prime - 9 * f * f_prime * f_second + f * f * f_third);
    }

    return t;
  }

  float quadratic_horizontal_coverage(vec2 pixelPos, float pixel_size) {
    // TODO: pass inv_pixel_size directly
    float inv_pixel_size = 1 / pixel_size;
    float coverage = 0 + float(uMinSamples + uMaxSamples) / 10000000.0;

    uint hBands = (vBandsData >> 28) + 1U;
    uint vBands = ((vBandsData >> 24) & 0xFU) + 1U;

    uvec2 minBandIndex = uvec2(
      clamp(uint(floor(pixelPos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixelPos.y * float(vBands))), 0U, vBands - 1U)
    );

    uvec2 maxBandIndex = uvec2(
      clamp(uint(floor(pixelPos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixelPos.x * float(vBands))), 0U, vBands - 1U)
    );

    uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
    uint yBandsOffset = vBandsData & 0xFFFU;
    uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
    uint yCurvesOffset = vCurvesData & 0xFFFU;

    uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curvesIndexOffset = xCurvesOffset + yCurvesOffset * 512U;

    uint hBandDataStart = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U)).x + bandsIndexOffset;
    uint hBandCurvesCount = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U + 1U)).x;

    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(hBandDataStart + curve)).x;

      vec4 p01 = (texture(uCurvesTexture, toCoords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize) 
        - vec4(pixelPos, pixelPos);
      vec2 p2 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)).xy - vPosition) / vSize 
        - pixelPos;
      
      vec2 p0 = p01.xy;
      vec2 p1 = p01.zw;

      bool is_downwards = p0.y > 0 || p2.y < 0;

      if (is_downwards) {
        // TODO: maybe inlining helps?
        if (p0.y < 0 && p2.y <= 0) continue;
        if (p0.y > 0 && p2.y >= 0) continue;
      } else {
        if (p0.y <= 0 && p2.y < 0) continue;
        if (p0.y >= 0 && p2.y > 0) continue;
      }

      vec2 a = p0 - 2 * p1 + p2;
      vec2 b = 2 * (p1 - p0);
      vec2 c = p0;

      float t = calculateQuadraticRoot(a.y, b.y, c.y);
      float intersect = a.x * t * t + b.x * t + c.x;

      coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (int(is_downwards) * 2 - 1);
    }

    return coverage;
  }

  float cubic_horizontal_coverage(vec2 pixelPos, float pixel_size) {
    float inv_pixel_size = 1 / pixel_size;
    float coverage = 0.0;

    uint hBands = (vBandsData >> 28) + 1U;
    uint vBands = ((vBandsData >> 24) & 0xFU) + 1U;

    uvec2 minBandIndex = uvec2(
      clamp(uint(floor(pixelPos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixelPos.y * float(vBands))), 0U, vBands - 1U)
    );

    uvec2 maxBandIndex = uvec2(
      clamp(uint(floor(pixelPos.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(pixelPos.x * float(vBands))), 0U, vBands - 1U)
    );

    uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
    uint yBandsOffset = vBandsData & 0xFFFU;
    uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
    uint yCurvesOffset = vCurvesData & 0xFFFU;

    uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curvesIndexOffset = xCurvesOffset + yCurvesOffset * 512U;

    uint hBandDataStart = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U)).x + bandsIndexOffset;
    uint hBandCurvesCount = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U + 1U)).x;

    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(hBandDataStart + curve)).x * 2U;

      vec4 p01 = (texture(uCurvesTexture, toCoords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize) 
        - vec4(pixelPos, pixelPos);
      vec4 p23 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)) 
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
        - vec4(pixelPos, pixelPos);

      vec2 p0 = p01.xy;
      vec2 p1 = p01.zw;
      vec2 p2 = p23.xy;
      vec2 p3 = p23.zw;

      bool is_downwards = p0.y > 0 || p3.y < 0;

      if (is_downwards) {
        if (p0.y < 0 && p3.y <= 0) continue; 
        if (p0.y > 0 && p3.y >= 0) continue;
      } else {
        if (p0.y <= 0 && p3.y < 0) continue;
        if (p0.y >= 0 && p3.y > 0) continue;
      }

      const float epsilon = 1e-7;

      bool b01 = abs(p1.x - p0.x) + abs(p1.y - p0.y) < epsilon;
      bool b23 = abs(p3.x - p2.x) + abs(p3.y - p2.y) < epsilon;
      bool b12 = abs(p2.x - p1.x) + abs(p2.y - p1.y) < epsilon;

      float intersect = 0.0;

      if ((b01 && (b23 || b12)) || (b23 && b12)) {
        vec2 a = p3 - p0;
        vec2 b = p0;
          
        float t = - b.y / a.y;

        intersect = a.x * t + b.x;
      } else {
        vec2 a = -p0 + 3 * p1 - 3 * p2 + p3;
        vec2 b = 3 * (p0 - 2 * p1 + p2);
        vec2 c = 3 * (p1 - p0);
        vec2 d = p0;

        float t0 = abs((-p0.y) / (p3.y - p0.y));
        float t = calculate_cubic_root(a.y, b.y, c.y, d.y, t0);
        float t_sq = t * t;

        intersect = a.x * t_sq * t + b.x * t_sq + c.x * t + d.x;
      }

      coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (int(is_downwards) * 2 - 1);
    }

    return coverage;
  }

  float quadratic_coverage() {
    vec2 pixel_size = vec2(fwidth(vTexCoord.x), fwidth(vTexCoord.y));
    float alphaSum = 0.0;

    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 samplePos = vTexCoord + vec2(0, yOffset) * pixel_size.y / 3.0;
      float coverage = quadratic_horizontal_coverage(samplePos, pixel_size.x);

      alphaSum += coverage;
    }

    return alphaSum / 3.0;
  }

  float cubic_coverage() {
    vec2 pixel_size = vec2(fwidth(vTexCoord.x), fwidth(vTexCoord.y));
    float alphaSum = 0.0;

    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 samplePos = vTexCoord + vec2(0, yOffset) * pixel_size.y / 3.0;
      float coverage = cubic_horizontal_coverage(samplePos, pixel_size.x);

      alphaSum += coverage;
    }

    return alphaSum / 3.0;
  }

  void main() {
    bool is_quadratic = bool(vCurvesData >> 28);
    bool is_even_odd = bool((vCurvesData >> 24) & 0xFU);
    
    // TODO: if this works out well, add linear coverage too.
    float coverage = is_quadratic ? quadratic_coverage() : cubic_coverage();
    float alpha = vColor.a;

    if (!is_even_odd) {
      alpha = alpha * abs(coverage - 2.0 * round(0.5 * coverage));
    } else {
      alpha = alpha * min(abs(coverage), 1.0);
    }

    oFragColor = vec4(vColor.rgb, 1.0) * alpha;
  }

)"
