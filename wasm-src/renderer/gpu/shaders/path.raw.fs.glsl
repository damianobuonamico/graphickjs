R"(

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

  const float kQuadraticEpsilon = 0.0001;
  
  vec2 toCoords(uint x) {
    return vec2(float(x % 512U) + 0.5, float(x / 512U) + 0.5) / 512.0;
  }

  // Calculate roots of quadratic equation(value/s for which: a×t ^ 2 + b×t + c = 0)
  vec2 calculateQuadraticRoots(float a, float b, float c) {
    const float epsilon = 1e-5;
    vec2 roots = vec2(-99999.0, -99999.0);

    // For a straight line, solve: b×t + c = 0; therefore t = -c/b
    if (abs(a) < epsilon) {
      if (b != 0) roots[0] = -c / b;
    } else {
      // Solve using quadratic formula: t = (-b ± sqrt(b^2 - 4ac)) / (2a)
      // If the value under the sqrt is negative, the equation has no real roots
      float discriminant = b * b - 4 * a * c;

      // Allow discriminant to be slightly negative to avoid a curve being missed due
      // to precision limitations. Must be clamped to zero before it's used in sqrt though!
      if (discriminant > -epsilon) {
        float s = sqrt(max(0, discriminant));
        roots[0] = (-b + s) / (2 * a);
        roots[1] = (-b - s) / (2 * a);
      }
    }

    return roots;
  }

  float calculateQuadraticRoot(float a, float b, float c) {
    const float epsilon = 1e-5;
    const float t_epsilon = 1e-4;

    // For a straight line, solve: b×t + c = 0; therefore t = -c/b
    if (abs(a) < epsilon) {
      if (b != 0) return -c / b;
    }

    // Solve using quadratic formula: t = (-b ± sqrt(b^2 - 4ac)) / (2a)
    // If the value under the sqrt is negative, the equation has no real roots
    float discriminant = b * b - 4 * a * c;

    // Allow discriminant to be slightly negative to avoid a curve being missed due
    // to precision limitations. Must be clamped to zero before it's used in sqrt though!
    if (discriminant > -epsilon) {
      float s = sqrt(max(0, discriminant));
      float t = (-b + s) / (2 * a);

      if (t >= -t_epsilon && t <= 1 + t_epsilon) return t;
      else return (-b - s) / (2 * a);
    }

    return -99999.0;
  }

  float calculateCubicRoot(float a, float b, float c, float d) {
    const float epsilon = 1e-5;
    
    if (abs(a) < epsilon) {
      return 999.99;
      if (c != 0) return -d / c;
    }

    float t = 0.5;

    for (int i = 0; i < 5; i++) {
      float t_sq = t * t;
      float f = a * t_sq * t + b * t_sq + c * t + d;
      float f_prime = 3 * a * t_sq + 2 * b * t + c;

      t = t - f / f_prime;
    }

    return t;
  }

  // Calculate the fraction [0,1] of the pixel that is covered by the glyph (along the x axis).
  // This is done by looking at the distances to the intersection points of a horizontal ray
  // (at the pixel pos) with all the curves of the glyph.
  float quadratic_horizontal_coverage(vec2 pixelPos, float pixelSize) {
    float coverage = 0 + float(uMinSamples + uMaxSamples) / 10000000.0;
    float invPixelSize = 1 / pixelSize;

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

    // Loop over all contours.
    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(hBandDataStart + curve)).x;

      // Get positions of curve's control points relative to the current pixel.
      vec4 p01 = (texture(uCurvesTexture, toCoords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize) 
        - vec4(pixelPos, pixelPos);
      vec2 p2 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)).xy - vPosition) / vSize 
        - pixelPos;
      vec2 p1 = p01.zw;
      vec2 p0 = p01.xy;

      // Check if curve segment is going downwards (this means that a ray crossing
      // it from left to right would be exiting the shape at this point).
      // Note: curves are assumed to be monotonic (strictly increasing or decreasing on the y axis).
      bool isDownwardCurve = p0.y > 0 || p2.y < 0;

      // Skip curves that are entirely above or below the ray.
      // When two curves are in the same direction (upward or downward), only one of them should be
      // counted at their meeting point to avoid double-counting. When in opposite directions, however,
      // the curve is not crossing the contour (but rather just grazing it) and so the curves should
      // either both be skipped, or both counted (so as not to affect the end result).
      if (isDownwardCurve) {
        if (p0.y < 0 && p2.y <= 0) continue;
        if (p0.y > 0 && p2.y >= 0) continue;
      } else {
        if (p0.y <= 0 && p2.y < 0) continue;
        if (p0.y >= 0 && p2.y > 0) continue;
      }

      // Calculate a,b,c of quadratic equation for current bezier curve.
      vec2 a = p0 - 2 * p1 + p2;
      vec2 b = 2 * (p1 - p0);
      vec2 c = p0;

      // Calculate roots to see if ray intersects curve segment.
      // Note: intersection is allowed slightly outside of [0, 1] segment to tolerate precision issues.
      const float epsilon = 1e-4;

      float root = calculateQuadraticRoot(a.y, b.y, c.y);

      if (root >= -epsilon && root <= 1 + epsilon) {
        // Calculate distance to intersection (negative if to left of ray)
        float t = clamp(root, 0.0, 1.0);
        float intersect = a.x * t * t + b.x * t + c.x;

        // Calculate the fraction of the ray that passes through the glyph (within the current pixel):
        // A value [0, 1] is calculated based on where the intersection occurs: 0 at the left edge of
        // the pixel, increasing to 1 at the right edge. This value is added to the total coverage
        // value when the ray exits a shape, and subtracted when the ray enters a shape.
        int sign_correction = isDownwardCurve ? 1 : -1;

        coverage += clamp(0.5 + intersect * invPixelSize, 0.0, 1.0) * sign_correction;
      }
    }

    return coverage;
  }

  float cubic_horizontal_coverage(vec2 pixelPos, float pixelSize) {
    float coverage = 0 + float(uMinSamples + uMaxSamples) / 10000000.0;
    float invPixelSize = 1 / pixelSize;

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

    // Loop over all contours
    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(hBandDataStart + curve)).x * 2U;

      // Get positions of curve's control points relative to the current pixel
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

      // Check if curve segment is going downwards (this means that a ray crossing
      // it from left to right would be exiting the shape at this point).
      // Note: curves are assumed to be monotonic (strictly increasing or decreasing on the y axis)
      bool isDownwardCurve = p0.y > 0 || p3.y < 0;

      // Skip curves that are entirely above or below the ray
      // When two curves are in the same direction (upward or downward), only one of them should be
      // counted at their meeting point to avoid double-counting. When in opposite directions, however,
      // the curve is not crossing the contour (but rather just grazing it) and so the curves should
      // either both be skipped, or both counted (so as not to affect the end result).
      if (isDownwardCurve) {
        if (p0.y < 0 && p3.y <= 0) continue; 
        if (p0.y > 0 && p3.y >= 0) continue;
      } else {
        if (p0.y <= 0 && p3.y < 0) continue;
        if (p0.y >= 0 && p3.y > 0) continue;
      }

      // Note: intersection is allowed slightly outside of [0, 1] segment to tolerate precision issues.
      const float epsilon = 1e-4;

      // vec2 a = p0 - 2 * p2 + p3;
      // vec2 b = 2 * (p2 - p0);
      // vec2 c = p0;

      // if (abs(p1.y - p3.y) < epsilon && abs(p2.y - p3.y) < epsilon) {
      //   float root = -p0.y / (p3.y - p0.y);

      //   if (root >= -epsilon && root <= 1 + epsilon) {
      //     float t = clamp(root, 0.0, 1.0);
      //     float t_sq = t * t;
      //     float intersect = (p3.x - p0.x) * t + p0.x;

      //     int sign_correction = isDownwardCurve ? 1 : -1;

      //     coverage += clamp(0.5 + intersect * invPixelSize, 0.0, 1.0) * sign_correction;
      //   }
      // } else {
        // Calculate a,b,c,d of cubic equation for current bezier curve
        vec2 a = -p0 + 3 * p1 - 3 * p2 + p3;
        vec2 b = 3 * (p0 - 2 * p1 + p2);
        vec2 c = 3 * (p1 - p0);
        vec2 d = p0;

        // Calculate roots to see if ray intersects curve segment.
        
        float root = calculateCubicRoot(a.y, b.y, c.y, d.y);
        // float root = calculateQuadraticRoot(a.y, b.y, c.y);

        if (root == 999.99) return 999.99;

        if (root >= -epsilon && root <= 1 + epsilon) {
          // Calculate distance to intersection (negative if to left of ray)
          float t = clamp(root, 0.0, 1.0);
          float t_sq = t * t;
          float intersect = a.x * t_sq * t + b.x * t_sq + c.x * t + d.x;
          // float intersect = a.x * t_sq + b.x * t + c.x;

          // Calculate the fraction of the ray that passes through the glyph (within the current pixel):
          // A value [0, 1] is calculated based on where the intersection occurs: 0 at the left edge of
          // the pixel, increasing to 1 at the right edge. This value is added to the total coverage
          // value when the ray exits a shape, and subtracted when the ray enters a shape.
          int sign_correction = isDownwardCurve ? 1 : -1;

          coverage += clamp(0.5 + intersect * invPixelSize, 0.0, 1.0) * sign_correction;
        }
      }
    // }

    return coverage;
  }

  float quadratic_coverage() {
    // Size of pixel in glyph space
    float pixelSize = fwidth(vTexCoord.x);
    float alphaSum = 0.0;

    // Render 3 times (with slight y offset) for anti-aliasing
    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 samplePos = vTexCoord + vec2(0, yOffset) * pixelSize / 3.0;
      float coverage = quadratic_horizontal_coverage(samplePos, pixelSize);

      if (coverage == 999.99) return 999.99;

      alphaSum += coverage;
    }

    return alphaSum / 3.0;
  }

  float cubic_coverage() {
    // Size of pixel in glyph space
    float pixelSize = fwidth(vTexCoord.x);
    float alphaSum = 0.0;

    // Render 3 times (with slight y offset) for anti-aliasing
    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 samplePos = vTexCoord + vec2(0, yOffset) * pixelSize / 3.0;
      float coverage = cubic_horizontal_coverage(samplePos, pixelSize);

      alphaSum += coverage;
    }

    return alphaSum / 3.0;
  }

  void main() {
    bool is_quadratic = bool(vCurvesData >> 28);
    bool is_even_odd = bool((vCurvesData >> 24) & 0xFU);

    // if (is_quadratic) {
    //   oFragColor = vec4(0.0, vTexCoord.y / 10.0, 0.0, 1.0);
    //   return;
    // }

    // TODO: if this works out well, add linear coverage too.
    float coverage = is_quadratic ? quadratic_coverage() : cubic_coverage();

    if (coverage == 999.99) {
      oFragColor = vec4(0.0, 1.0, 0.0, 1.0);
      return;
    }

    if (!is_even_odd) {
      float alpha = vColor.a * abs(coverage - 2.0 * round(0.5 * coverage));
      oFragColor = vec4(vColor.rgb, 1.0) * alpha;
    } else {
      float alpha = vColor.a * min(abs(coverage), 1.0);
      oFragColor = vec4(vColor.rgb, 1.0) * alpha;
    }
  }

)"
