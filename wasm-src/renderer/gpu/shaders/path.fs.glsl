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

  void main() {
    float coverage = 0.0;

    vec2 pixelSize = vec2(fwidth(vTexCoord.x), fwidth(vTexCoord.y));
    vec2 pixelsPerEm = 1.0 / pixelSize;

    uint hBands = (vBandsData >> 28) + 1U;
    uint vBands = ((vBandsData >> 24) & 0xFU) + 1U;

    uvec2 minBandIndex = uvec2(
      clamp(uint(floor((vTexCoord.y - 0.49 * pixelSize.y) * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor((vTexCoord.x - 0.49 * pixelSize.x) * float(vBands))), 0U, vBands - 1U)
    );

    uvec2 maxBandIndex = uvec2(
      clamp(uint(floor((vTexCoord.y + 0.49 * pixelSize.y) * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor((vTexCoord.x + 0.49 * pixelSize.x) * float(vBands))), 0U, vBands - 1U)
    );

    uvec2 sampleCount = clamp(
      uvec2(32.0 / pixelsPerEm + 1.0), 
      uvec2(uMinSamples), uvec2(uMaxSamples)
    );

    bool hBandChanged = false;

    if (minBandIndex.x != maxBandIndex.x) {
      minBandIndex.x = clamp(uint(floor(vTexCoord.y * float(hBands))), 0U, hBands - 1U);
      sampleCount.x = 1U;
      sampleCount.y *= 2U;

      hBandChanged = true;
    }

    if (minBandIndex.y != maxBandIndex.y) {
      minBandIndex.y = clamp(uint(floor(vTexCoord.x * float(vBands))), 0U, vBands - 1U);
      sampleCount.y = 1U;

      if (!hBandChanged) sampleCount.x *= 2U;
    }

    uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
    uint yBandsOffset = vBandsData & 0xFFFU;
    uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
    uint yCurvesOffset = vCurvesData & 0xFFFU;

    uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curvesIndexOffset = xCurvesOffset + yCurvesOffset * 512U;

    float hSampleDelta = 1.0 / float(2U * sampleCount.x) * pixelSize.y;
    float vSampleDelta = 1.0 / float(2U * sampleCount.y) * pixelSize.x;

    uint hBandDataStart = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U)).x + bandsIndexOffset;
    uint hBandCurvesCount = texture(uBandsTexture, toCoords(bandsIndexOffset + minBandIndex.x * 2U + 1U)).x;

    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(hBandDataStart + curve)).x;

      vec4 p12 = (texture(uCurvesTexture, toCoords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
        - vec4(vTexCoord, vTexCoord)
        - vec4(0.0, 0.5 * pixelSize.y - hSampleDelta, 0.0, 0.5 * pixelSize.y - hSampleDelta);

      vec2 p3 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)).xy 
        - vPosition) / vSize 
        - vTexCoord
        - vec2(0.0, 0.5 * pixelSize.y - hSampleDelta);

      if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) break;

      uint code = (0x2E74U >> (((p12.y > 0.0) ? 2U : 0U) + ((p12.w > 0.0) ? 4U : 0U) + ((p3.y > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        for (uint iSample = 0U; iSample < sampleCount.x; iSample++) {
          float ax = p12.x - p12.z * 2.0 + p3.x;
          float ay = p12.y - p12.w * 2.0 + p3.y;
          float bx = p12.x - p12.z;
          float by = p12.y - p12.w;
          float ra = 1.0 / ay;

          float d = sqrt(max(by * by - ay * p12.y, 0.0));
          float t1 = (by - d) * ra;
          float t2 = (by + d) * ra;
        
          if (abs(ay) < kQuadraticEpsilon) t1 = t2 = p12.y * 0.5 / by;

          float x1 = (ax * t1 - bx * 2.0) * t1 + p12.x;
          float x2 = (ax * t2 - bx * 2.0) * t2 + p12.x;
          x1 = clamp(x1 * pixelsPerEm.x + 0.5, 0.0, 1.0);
          x2 = clamp(x2 * pixelsPerEm.x + 0.5, 0.0, 1.0);

          if ((code & 1U) != 0U) coverage += x1;
          if (code > 1U) coverage -= x2;

          p12 += vec4(0.0, 2.0 * hSampleDelta, 0.0, 2.0 * hSampleDelta);
          p3 += vec2(0.0, 2.0 * hSampleDelta);
        }
      }
    }

    uint vBandDataStart = texture(uBandsTexture, toCoords(bandsIndexOffset + hBands * 2U + minBandIndex.y * 2U)).x + bandsIndexOffset;
    uint vBandCurvesCount = texture(uBandsTexture, toCoords(bandsIndexOffset + hBands * 2U + minBandIndex.y * 2U + 1U)).x;

    for (uint curve = 0U; curve < vBandCurvesCount; curve++) {
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, toCoords(vBandDataStart + curve)).x;

      vec4 p12 = (texture(uCurvesTexture, toCoords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
        - vec4(vTexCoord, vTexCoord)
        - vec4(0.5 * pixelSize.x - vSampleDelta, 0.0, 0.5 * pixelSize.x - vSampleDelta, 0.0);

      vec2 p3 = (texture(uCurvesTexture, toCoords(curveOffset + 1U)).xy 
        - vPosition) / vSize 
        - vTexCoord
        - vec2(0.5 * pixelSize.x - vSampleDelta, 0.0);

      if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) break;

      uint code = (0x2E74U >> (((p12.x > 0.0) ? 2U : 0U) + ((p12.z > 0.0) ? 4U : 0U) + ((p3.x > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        for (uint iSample = 0U; iSample < sampleCount.y; iSample++) {
          float ax = p12.y - p12.w * 2.0 + p3.y;
          float ay = p12.x - p12.z * 2.0 + p3.x;
          float bx = p12.y - p12.w;
          float by = p12.x - p12.z;
          float ra = 1.0 / ay;

          float d = sqrt(max(by * by - ay * p12.x, 0.0));
          float t1 = (by - d) * ra;
          float t2 = (by + d) * ra;

          if (abs(ay) < kQuadraticEpsilon) t1 = t2 = p12.x * 0.5 / by;

          float x1 = (ax * t1 - bx * 2.0) * t1 + p12.y;
          float x2 = (ax * t2 - bx * 2.0) * t2 + p12.y;
          x1 = clamp(x1 * pixelsPerEm.y + 0.5, 0.0, 1.0);
          x2 = clamp(x2 * pixelsPerEm.y + 0.5, 0.0, 1.0);

          if ((code & 1U) != 0U) coverage -= x1;
          if (code > 1U) coverage += x2;

          p12 += vec4(2.0 * vSampleDelta, 0.0, 2.0 * vSampleDelta, 0.0);
          p3 += vec2(2.0 * vSampleDelta, 0.0);
        }
      }
    }

    coverage = sqrt(clamp(abs(coverage / float(sampleCount.x + sampleCount.y)), 0.0, 1.0));

    float alpha = vColor.a * coverage;

    oFragColor = vec4(vColor.rgb, 1.0) * alpha;
  }

)"
