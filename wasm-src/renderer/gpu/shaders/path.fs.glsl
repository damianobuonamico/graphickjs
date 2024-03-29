R"(

  precision highp float;
  precision highp sampler2D;
  precision mediump usampler2D;

  uniform sampler2D uCurvesTexture;
  uniform usampler2D uBandsTexture;

  in vec4 vColor;
  in vec2 vTexCoord;

  flat in vec2 vPosition;
  flat in vec2 vSize;
  flat in uint vCurvesData;
  flat in uint vBandsData;

  out vec4 oFragColor;

  const float kQuadraticEpsilon = 0.0001;
  
  vec2 to_coords(uint x, uint y) {
    return vec2(float(x) + 0.5, float(y) + 0.5) / 512.0;
  }

  void main() {
    float coverage = 0.0;

    /* The effective pixel dimensions of the em square. */
	  vec2 pixelsPerEm = vec2(1.0 / fwidth(vTexCoord.x), 1.0 / fwidth(vTexCoord.y));
    vec2 pixelSize = 1.0 / pixelsPerEm;

    // TODO: max samples uniform
    ivec2 sampleCount = ivec2(4, 4);

    uint hBands = (vBandsData >> 28) + 1U;
    uint vBands = ((vBandsData >> 24) & 0xFU) + 1U;
    uint xBandsOffset = (vBandsData >> 12) & 0xFFFU;
    uint yBandsOffset = vBandsData & 0xFFFU;
    uint xCurvesOffset = (vCurvesData >> 12) & 0xFFFU;
    uint yCurvesOffset = vCurvesData & 0xFFFU;
 
    uvec2 bandIndex = uvec2(
      floor(vTexCoord.y * float(hBands)),
      floor(vTexCoord.x * float(vBands))
    );

    if (bandIndex.x >= hBands || bandIndex.y >= vBands) {
      oFragColor = vec4(0.0, 0.0, 0.0, 0.0);
      return;
    }

    uint hBandDataStart = texture(uBandsTexture, to_coords(xBandsOffset + bandIndex.x * 2U, yBandsOffset)).x + xBandsOffset;
    uint hBandCurvesCount = texture(uBandsTexture, to_coords(xBandsOffset + bandIndex.x * 2U + 1U, yBandsOffset)).x;

    for (uint curve = 0U; curve < hBandCurvesCount; curve++) {
      uint curveOffset = texture(uBandsTexture, to_coords(hBandDataStart + curve, yBandsOffset)).x;
      uint xCurve = xCurvesOffset + curveOffset % 512U;
      uint yCurve = yCurvesOffset + curveOffset / 512U;

      vec4 p12 = (texture(uCurvesTexture, to_coords(xCurve, yCurve))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
        - vec4(vTexCoord.x - 0.5 * pixelSize.x, vTexCoord.y, vTexCoord.x - 0.5 * pixelSize.x, vTexCoord.y);
        // - vec4(vTexCoord, vTexCoord);
        // - vec4(vTexCoord.x, vTexCoord.y + 0.5 / pixelsPerEm.y, vTexCoord.x, vTexCoord.y + 0.5 / pixelsPerEm.y);
      vec2 p3 = (texture(uCurvesTexture, to_coords(xCurve + 1U, yCurve)).xy 
        - vPosition) / vSize 
        - vec2(vTexCoord.x - 0.5 * pixelSize.x, vTexCoord.y);
        // - vTexCoord;
        // - vec2(vTexCoord.x, vTexCoord.y + 0.5 / pixelsPerEm.y);

      if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) continue;

      uint code = (0x2E74U >> (((p12.y > 0.0) ? 2U : 0U) + ((p12.w > 0.0) ? 4U : 0U) + ((p3.y > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        for (int sample_i = 0; sample_i < sampleCount.y; sample_i++) {
          float sample_delta = float(1 + 2 * sample_i) / float(2 * sampleCount.y) * pixelSize.y;

          p12 += vec4(0.0, sample_delta, 0.0, sample_delta);
          p3 += vec2(0.0, sample_delta);

          // p12 += vec4(0.0, float(sample_i + 1) / (float(sampleCount.y + 1) * pixelsPerEm.y), 0.0, float(sample_i + 1) / (float(sampleCount.y + 1) * pixelsPerEm.y));
          // p3 += vec2(0.0, float(sample_i + 1) / (float(sampleCount.y + 1) * pixelsPerEm.y));

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

          if ((code & 1U) != 0U) coverage += x1 / float(sampleCount.y);
          if (code > 1U) coverage -= x2 / float(sampleCount.y);
        }
      }
    }

    uint vBandDataStart = texture(uBandsTexture, to_coords(xBandsOffset + hBands * 2U + bandIndex.y * 2U, yBandsOffset)).x + xBandsOffset;
    uint vBandCurvesCount = texture(uBandsTexture, to_coords(xBandsOffset + hBands * 2U + bandIndex.y * 2U + 1U, yBandsOffset)).x;

    // for (uint curve = 0U; curve < 0U; curve++) {
    for (uint curve = 0U; curve < vBandCurvesCount; curve++) {
      uint curveOffset = texture(uBandsTexture, to_coords(vBandDataStart + curve, yBandsOffset)).x;
      uint xCurve = xCurvesOffset + curveOffset % 512U;
      uint yCurve = yCurvesOffset + curveOffset / 512U;

      vec4 p12 = (texture(uCurvesTexture, to_coords(xCurve, yCurve))
        - vec4(vPosition, vPosition)) / vec4(vSize, vSize)
        - vec4(vTexCoord.x, vTexCoord.y - 0.5 * pixelSize.y, vTexCoord.x, vTexCoord.y - 0.5 * pixelSize.y);
        // - vec4(vTexCoord, vTexCoord);
        // - vec4(vTexCoord.x + 0.5 / pixelsPerEm.x, vTexCoord.y, vTexCoord.x + 0.5 / pixelsPerEm.x, vTexCoord.y);
      vec2 p3 = (texture(uCurvesTexture, to_coords(xCurve + 1U, yCurve)).xy 
        - vPosition) / vSize 
        - vec2(vTexCoord.x, vTexCoord.y - 0.5 * pixelSize.y);
        // - vTexCoord;
        // - vec2(vTexCoord.x + 0.5 / pixelsPerEm.x, vTexCoord.y);

      if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) break;

      uint code = (0x2E74U >> (((p12.x > 0.0) ? 2U : 0U) + ((p12.z > 0.0) ? 4U : 0U) + ((p3.x > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        for (int sample_i = 0; sample_i < sampleCount.x; sample_i++) {
          float sample_delta = float(1 + 2 * sample_i) / float(2 * sampleCount.x) * pixelSize.x;

          p12 += vec4(sample_delta, 0.0, sample_delta, 0.0);
          p3 += vec2(sample_delta, 0.0);
          
          // p12 += vec4(float(sample_i + 1) / (float(sampleCount.x + 1) * pixelsPerEm.x), 0.0, float(sample_i + 1) / (float(sampleCount.x + 1) * pixelsPerEm.x), 0.0);
          // p3 += vec2(float(sample_i + 1) / (float(sampleCount.x + 1) * pixelsPerEm.x), 0.0);

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

          if ((code & 1U) != 0U) coverage -= x1 / float(sampleCount.x);
          if (code > 1U) coverage += x2 / float(sampleCount.x);
        }
      }
    }

    coverage = sqrt(clamp(abs(coverage * 0.5), 0.0, 1.0));
	  float alpha = coverage * vColor.a;
	  // oFragColor = vec4(vColor.rgb * alpha, alpha);
    
	  oFragColor = vec4(vColor.rgb * vec3(float(bandIndex.x + 1U) / float(hBands - 1U), float(bandIndex.y + 1U) / float(vBands - 1U), 0.0) * alpha, alpha);


    // oFragColor = vColor * 0.0000000001 + vec4(float(vBandCurvesCount) / 6.0, 0.0, 0.0, 1.0);
    // return;

    // oFragColor = vColor * vec4(float(bandIndex.x + 1U) / float(hBands - 1U), 1.0, 0.2, 1.0);
    // oFragColor *= vec4(1.0, float(bandIndex.y + 1U) / float(vBands - 1U), 1.0, 1.0);
  }

)"
