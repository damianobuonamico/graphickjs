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
  
  vec2 to_coords(uint x) {
    return vec2(float(x % 512U) + 0.5, float(x / 512U) + 0.5) / 512.0;
  }

  bool not_normalized(float x, float pixelSize) {
    return x <= -0.5 * pixelSize || x >= 1.0 + 0.5 * pixelSize;
  }

  void main() {
    vec2 texCoords = vTexCoord;
 
    // vec2 texCoords = vec2(
    //   clamp(vTexCoord.x, 0.01, 0.99),
    //   clamp(vTexCoord.y, 0.01, 0.99)
    // );

    float coverage = 0.0;

    /* The effective pixel dimensions of the em square. */
	  vec2 pixelsPerEm = vec2(1.0 / fwidth(texCoords.x), 1.0 / fwidth(texCoords.y));
    vec2 pixelSize = 1.0 / pixelsPerEm;

    // if (not_normalized(texCoords.x, pixelSize.x) || not_normalized(texCoords.y, pixelSize.y)) {
      // oFragColor = vec4(0.0, 1.0, 0.0, 1.0);
      // return;
      // discard;
    // }
    
    // ivec2 sampleCount = clamp(ivec2(32.0 / pixelsPerEm + 1.0), ivec2(uMinSamples, uMinSamples), ivec2(uMaxSamples, uMaxSamples));
    ivec2 sampleCount = ivec2(min(2, uMinSamples), min(0, uMaxSamples));

    uint hBands = (vBandsData >> 28) + 1U;
    uint vBands = ((vBandsData >> 24) & 0xFU) + 1U;

    uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
    uint yBandsOffset = vBandsData & 0xFFFU;
    uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
    uint yCurvesOffset = vCurvesData & 0xFFFU;

    uint bandsIndexOffset = xBandsOffset + yBandsOffset * 512U;
    uint curvesIndexOffset = xCurvesOffset + yCurvesOffset * 512U;

    uvec2 minBandIndex = uvec2(
      clamp(uint(floor((texCoords.y - 0.5 * pixelSize.y) * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor((texCoords.x - 0.5 * pixelSize.x) * float(vBands))), 0U, vBands - 1U)
    );

    uvec2 maxBandIndex = uvec2(
      clamp(uint(floor((texCoords.y + 0.5 * pixelSize.y) * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor((texCoords.x + 0.5 * pixelSize.x) * float(vBands))), 0U, vBands - 1U)
    );

    if (minBandIndex != maxBandIndex) {
      oFragColor = vec4(0.0, 1.0, 0.0, 1.0);
      return;
    }

    uvec2 bandIndex = uvec2(
      clamp(uint(floor(texCoords.y * float(hBands))), 0U, hBands - 1U),
      clamp(uint(floor(texCoords.x * float(vBands))), 0U, vBands - 1U)
    );
   
    float hSampleDelta = 1.0 / float(2 * sampleCount.x) * pixelSize.y;
    float vSampleDelta = 1.0 / float(2 * sampleCount.y) * pixelSize.x;

    uint hBandDataStart = texture(uBandsTexture, to_coords(bandsIndexOffset + bandIndex.x * 2U)).x + bandsIndexOffset;
    uint hBandCurvesCount = texture(uBandsTexture, to_coords(bandsIndexOffset + bandIndex.x * 2U + 1U)).x;

    // if (hBandDataStart > 512U) {
    //   hBandDataStart %= 512U;
    //   yBandsOffset += 1U;
    // }

    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curveOffset = bandsIndexOffset + texture(uBandsTexture, to_coords(hBandDataStart + curve)).x;

      vec4 p12 = (texture(uCurvesTexture, to_coords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
        - vec4(texCoords, texCoords)
        - vec4(0.0, 0.5 * pixelSize.y - hSampleDelta, 0.0, 0.5 * pixelSize.y - hSampleDelta);

      vec2 p3 = (texture(uCurvesTexture, to_coords(curveOffset + 1U)).xy 
        - vPosition) / vSize 
        - texCoords
        - vec2(0.0, 0.5 * pixelSize.y - hSampleDelta);

      if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) continue;

      uint code = (0x2E74U >> (((p12.y > 0.0) ? 2U : 0U) + ((p12.w > 0.0) ? 4U : 0U) + ((p3.y > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        for (int sample_i = 0; sample_i < sampleCount.x; sample_i++) {
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

    uint vBandDataStart = texture(uBandsTexture, to_coords(bandsIndexOffset + hBands * 2U + bandIndex.y * 2U)).x + bandsIndexOffset;
    uint vBandCurvesCount = texture(uBandsTexture, to_coords(bandsIndexOffset + hBands * 2U + bandIndex.y * 2U + 1U)).x;

    // if (vBandDataStart > 512U) {
    //   vBandDataStart %= 512U;
    //   yBandsOffset += 1U;
    // }

    for (uint curve = 0U; curve < vBandCurvesCount; curve++) {
      uint curveOffset = curvesIndexOffset + texture(uBandsTexture, to_coords(vBandDataStart + curve)).x;

      vec4 p12 = (texture(uCurvesTexture, to_coords(curveOffset))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
        - vec4(texCoords, texCoords)
        - vec4(0.5 * pixelSize.x - vSampleDelta, 0.0, 0.5 * pixelSize.x - vSampleDelta, 0.0);

      vec2 p3 = (texture(uCurvesTexture, to_coords(curveOffset + 1U)).xy 
        - vPosition) / vSize 
        - texCoords
        - vec2(0.5 * pixelSize.x - vSampleDelta, 0.0);

      if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) break;

      uint code = (0x2E74U >> (((p12.x > 0.0) ? 2U : 0U) + ((p12.z > 0.0) ? 4U : 0U) + ((p3.x > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        for (int sample_i = 0; sample_i < sampleCount.y; sample_i++) {
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
    // if (coverage == 0.0) {
    //   oFragColor = vec4(1.0, 0.0, 0.0, 1.0);
    //   return;
    // }
	  float alpha = vColor.a * coverage;
	  // float alpha = coverage + 0.2;

    // if (texCoords.x < 0.0 || texCoords.x > 1.0 || texCoords.y < 0.0 || texCoords.y > 1.0) {
    //   oFragColor = vec4(0.0, 1.0, 0.0, 1.0);
    //   return;
    // }

	  // oFragColor = vec4(vColor.rgb * 0.0000000000001 + vec3(1.0, 1.0, 1.0) * alpha, alpha);
	  // oFragColor = vec4((vColor.rgb * 0.00000000001 + vec3(texCoords, 0.0)) * alpha, alpha);

	  // oFragColor = vec4(vColor.rgb * 0.000000001 +  vec3(float(bandIndex.y) / float(hBands - 1U), 0.0, 0.0) * alpha, alpha);
    // oFragColor = vec4(vec3(sampleCount.x / 16.0) * alpha, alpha);
    // oFragColor = vec4(vColor.rgb * 0.000000001 + vec3(texCoords, 0.0) * alpha, alpha);
    oFragColor = vec4(vColor.rgb * alpha, alpha);


    // oFragColor = vColor * 0.0000000001 + vec4(float(vBandCurvesCount) / 6.0, 0.0, 0.0, 1.0);
    // return;

    // oFragColor = vColor * vec4(float(bandIndex.x + 1U) / float(hBands - 1U), 1.0, 0.2, 1.0);
    // oFragColor *= vec4(1.0, float(bandIndex.y + 1U) / float(vBands - 1U), 1.0, 1.0);
  }

)"
