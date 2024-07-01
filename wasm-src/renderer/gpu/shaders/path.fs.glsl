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

  // Calculate the fraction [0,1] of the pixel that is covered by the glyph (along the x axis).
  // This is done by looking at the distances to the intersection points of a horizontal ray
  // (at the pixel pos) with all the curves of the glyph.
  float calculateHorizontalCoverage(vec2 pixelPos, float pixelSize) {
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
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(hBandDataStart + curve)).x;

      // Get positions of curve's control points relative to the current pixel
      vec4 p01 = (texture(uCurvesTexture, toCoords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize) 
        - vec4(pixelPos, pixelPos);
      vec2 p2 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)).xy - vPosition) / vSize 
        - pixelPos;
      vec2 p1 = p01.zw;
      vec2 p0 = p01.xy;

      // Check if curve segment is going downwards (this means that a ray crossing
      // it from left to right would be exiting the shape at this point).
      // Note: curves are assumed to be monotonic (strictly increasing or decreasing on the y axis)
      bool isDownwardCurve = p0.y > 0 || p2.y < 0;

      // Skip curves that are entirely above or below the ray
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

      // Calculate a,b,c of quadratic equation for current bezier curve
      vec2 a = p0 - 2 * p1 + p2;
      vec2 b = 2 * (p1 - p0);
      vec2 c = p0;

      // Calculate roots to see if ray intersects curve segment.
      // Note: intersection is allowed slightly outside of [0, 1] segment to tolerate precision issues.
      const float epsilon = 1e-4;
      vec2 roots = calculateQuadraticRoots(a.y, b.y, c.y);
      bool onSeg0 = roots[0] >= -epsilon && roots[0] <= 1 + epsilon;
      bool onSeg1 = roots[1] >= -epsilon && roots[1] <= 1 + epsilon;

      // Calculate distance to intersection (negative if to left of ray)
      float t0 = clamp(roots[0], 0.0, 1.0);
      float t1 = clamp(roots[1], 0.0, 1.0);
      float intersect0 = a.x * t0 * t0 + b.x * t0 + c.x;
      float intersect1 = a.x * t1 * t1 + b.x * t1 + c.x;

      // Calculate the fraction of the ray that passes through the glyph (within the current pixel):
      // A value [0, 1] is calculated based on where the intersection occurs: 0 at the left edge of
      // the pixel, increasing to 1 at the right edge. This value is added to the total coverage
      // value when the ray exits a shape, and subtracted when the ray enters a shape.
      int signCorrection = isDownwardCurve ? 1 : -1;
      if (onSeg0) coverage += clamp(0.5 + intersect0 * invPixelSize, 0.0, 1.0) * signCorrection;
      if (onSeg1) coverage += clamp(0.5 + intersect1 * invPixelSize, 0.0, 1.0) * signCorrection;

      // pointOffset += numPoints + 1; 
    }

    return clamp(coverage, 0.0, 1.0);
  }

  void main() {
    // Size of pixel in glyph space
    float pixelSize = fwidth(vTexCoord.x);
    float alphaSum = 0.0;

    // Render 3 times (with slight y offset) for anti-aliasing
    for (int yOffset = -1; yOffset <= 1; yOffset++) {
      vec2 samplePos = vTexCoord + vec2(0, yOffset) * pixelSize / 3.0;
      float coverage = calculateHorizontalCoverage(samplePos, pixelSize);

      alphaSum += coverage;
    }

    float alpha = alphaSum / 3.0;

    oFragColor = vColor * alpha;
  }

  // void main() {
  //   float coverage = 0.0;

  //   vec2 pixelSize = vec2(fwidth(vTexCoord.x), fwidth(vTexCoord.y));
  //   vec2 pixelsPerEm = 1.0 / pixelSize;

  //   uint hBands = (vBandsData >> 28) + 1U;
  //   uint vBands = ((vBandsData >> 24) & 0xFU) + 1U;

  //   uvec2 minBandIndex = uvec2(
  //     clamp(uint(floor((vTexCoord.y - 0.49 * pixelSize.y) * float(hBands))), 0U, hBands - 1U),
  //     clamp(uint(floor((vTexCoord.x - 0.49 * pixelSize.x) * float(vBands))), 0U, vBands - 1U)
  //   );

  //   uvec2 maxBandIndex = uvec2(
  //     clamp(uint(floor((vTexCoord.y + 0.49 * pixelSize.y) * float(hBands))), 0U, hBands - 1U),
  //     clamp(uint(floor((vTexCoord.x + 0.49 * pixelSize.x) * float(vBands))), 0U, vBands - 1U)
  //   );

  //   uvec2 sampleCount = clamp(
  //     uvec2(32.0 / pixelsPerEm + 1.0), 
  //     uvec2(uMinSamples), uvec2(uMaxSamples)
  //   );

  //   bool hBandChanged = false;

  //   if (minBandIndex.x != maxBandIndex.x) {
  //     minBandIndex.x = clamp(uint(floor(vTexCoord.y * float(hBands))), 0U, hBands - 1U);
  //     sampleCount.x = 1U;
  //     sampleCount.y *= 2U;

  //     hBandChanged = true;
  //   }

  //   if (minBandIndex.y != maxBandIndex.y) {
  //     minBandIndex.y = clamp(uint(floor(vTexCoord.x * float(vBands))), 0U, vBands - 1U);
  //     sampleCount.y = 1U;

  //     if (!hBandChanged) sampleCount.x *= 2U;
  //   }

  //   uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
  //   uint yBandsOffset = vBandsData & 0xFFFU;
  //   uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
  //   uint yCurvesOffset = vCurvesData & 0xFFFU;

  //   uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
  //   uint curvesIndexOffset = xCurvesOffset + yCurvesOffset * 512U;

  //   float hSampleDelta = 1.0 / float(2U * sampleCount.x) * pixelSize.y;
  //   float vSampleDelta = 1.0 / float(2U * sampleCount.y) * pixelSize.x;

  //   uint hBandDataStart = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U)).x + bandsIndexOffset;
  //   uint hBandCurvesCount = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U + 1U)).x;

  //   for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
  //     uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(hBandDataStart + curve)).x;

  //     vec4 p12 = (texture(uCurvesTexture, toCoords(curveOffset))
  //       - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
  //       - vec4(vTexCoord, vTexCoord)
  //       - vec4(0.0, 0.5 * pixelSize.y - hSampleDelta, 0.0, 0.5 * pixelSize.y - hSampleDelta);

  //     vec2 p3 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)).xy 
  //       - vPosition) / vSize 
  //       - vTexCoord
  //       - vec2(0.0, 0.5 * pixelSize.y - hSampleDelta);

  //     if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) break;

  //     uint code = (0x2E74U >> (((p12.y > 0.0) ? 2U : 0U) + ((p12.w > 0.0) ? 4U : 0U) + ((p3.y > 0.0) ? 8U : 0U))) & 3U;

  //     if (code != 0U) {
  //       for (uint iSample = 0U; iSample < sampleCount.x; iSample++) {
  //         float ax = p12.x - p12.z * 2.0 + p3.x;
  //         float ay = p12.y - p12.w * 2.0 + p3.y;
  //         float bx = p12.x - p12.z;
  //         float by = p12.y - p12.w;
  //         float ra = 1.0 / ay;

  //         float d = sqrt(max(by * by - ay * p12.y, 0.0));
  //         float t1 = (by - d) * ra;
  //         float t2 = (by + d) * ra;
        
  //         if (abs(ay) < kQuadraticEpsilon) t1 = t2 = p12.y * 0.5 / by;

  //         float x1 = (ax * t1 - bx * 2.0) * t1 + p12.x;
  //         float x2 = (ax * t2 - bx * 2.0) * t2 + p12.x;
  //         x1 = clamp(x1 * pixelsPerEm.x + 0.5, 0.0, 1.0);
  //         x2 = clamp(x2 * pixelsPerEm.x + 0.5, 0.0, 1.0);

  //         if ((code & 1U) != 0U) coverage += x1;
  //         if (code > 1U) coverage -= x2;

  //         p12 += vec4(0.0, 2.0 * hSampleDelta, 0.0, 2.0 * hSampleDelta);
  //         p3 += vec2(0.0, 2.0 * hSampleDelta);
  //       }
  //     }
  //   }

  //   uint vBandDataStart = texture(uBandsTexture, toCoords(bandsIndexOffset + hBands * 2U + minBandIndex.y * 2U)).x + bandsIndexOffset;
  //   uint vBandCurvesCount = texture(uBandsTexture, toCoords(bandsIndexOffset + hBands * 2U + minBandIndex.y * 2U + 1U)).x;

  //   for (uint curve = 0U; curve < vBandCurvesCount; curve++) {
  //     uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(vBandDataStart + curve)).x;

  //     vec4 p12 = (texture(uCurvesTexture, toCoords(curveOffset))
  //       - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
  //       - vec4(vTexCoord, vTexCoord)
  //       - vec4(0.5 * pixelSize.x - vSampleDelta, 0.0, 0.5 * pixelSize.x - vSampleDelta, 0.0);

  //     vec2 p3 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)).xy 
  //       - vPosition) / vSize 
  //       - vTexCoord
  //       - vec2(0.5 * pixelSize.x - vSampleDelta, 0.0);

  //     if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) break;

  //     uint code = (0x2E74U >> (((p12.x > 0.0) ? 2U : 0U) + ((p12.z > 0.0) ? 4U : 0U) + ((p3.x > 0.0) ? 8U : 0U))) & 3U;

  //     if (code != 0U) {
  //       for (uint iSample = 0U; iSample < sampleCount.y; iSample++) {
  //         float ax = p12.y - p12.w * 2.0 + p3.y;
  //         float ay = p12.x - p12.z * 2.0 + p3.x;
  //         float bx = p12.y - p12.w;
  //         float by = p12.x - p12.z;
  //         float ra = 1.0 / ay;

  //         float d = sqrt(max(by * by - ay * p12.x, 0.0));
  //         float t1 = (by - d) * ra;
  //         float t2 = (by + d) * ra;

  //         if (abs(ay) < kQuadraticEpsilon) t1 = t2 = p12.x * 0.5 / by;

  //         float x1 = (ax * t1 - bx * 2.0) * t1 + p12.y;
  //         float x2 = (ax * t2 - bx * 2.0) * t2 + p12.y;
  //         x1 = clamp(x1 * pixelsPerEm.y + 0.5, 0.0, 1.0);
  //         x2 = clamp(x2 * pixelsPerEm.y + 0.5, 0.0, 1.0);

  //         if ((code & 1U) != 0U) coverage -= x1;
  //         if (code > 1U) coverage += x2;

  //         p12 += vec4(2.0 * vSampleDelta, 0.0, 2.0 * vSampleDelta, 0.0);
  //         p3 += vec2(2.0 * vSampleDelta, 0.0);
  //       }
  //     }
  //   }

  //   coverage = sqrt(clamp(abs(coverage / float(sampleCount.x + sampleCount.y)), 0.0, 1.0));

  //   float alpha = vColor.a * coverage;

  //   oFragColor = vec4(vColor.rgb, 1.0) * alpha;
  // }

)"
