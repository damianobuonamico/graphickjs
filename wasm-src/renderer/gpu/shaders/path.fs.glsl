R"(

  precision highp float;
  precision highp sampler2D;

  uniform sampler2D uCurvesTexture;

  in vec4 vColor;
  in vec2 vTexCoord;

  flat in vec2 vPosition;
  flat in vec2 vSize;

  out vec4 oFragColor;

  const float kQuadraticEpsilon = 0.0001;

  void main() {
	  float coverage = 0.0;

    /* The effective pixel dimensions of the em square. */
	  vec2 pixelsPerEm = vec2(1.0 / fwidth(vTexCoord.x), 1.0 / fwidth(vTexCoord.y));

    // TODO: max samples uniform
    ivec2 sampleCount = clamp(ivec2(32.0 / pixelsPerEm + 1.0), ivec2(4, 4), ivec2(8, 8));

    // uvec2 glyphLoc = uvec2(0U, 0U);

    for (uint curve = 0U; curve < 2U; curve++) {
      vec2 curveLoc = vec2(float(curve), 0.0);

      /* Fetch the three 2D control points for the current curve from the
       * curve texture. The first texel contains both p1 and p2 in the
       * (x,y) and (z,w) components, respectively, and the the second texel
       * contains p3 in the (x,y) components. The quadratic Bézier curve
       * C(t) is given by:
       *
       *     C(t) = (1 - t)^2 p1 + 2t(1 - t) p2 + t^2 p3
       */

      // vec4 p12 = (texture(uCurvesTexture, curveLoc / 512.0) - vec4(vPosition, vPosition)) / vec4(vSize, vSize) - vec4(vTexCoord - 0.5 / pixelsPerEm, vTexCoord - 0.5 / pixelsPerEm);
      // vec2 p3 = (texture(uCurvesTexture, vec2(curveLoc.x + 1, curveLoc.y) / 512.0).xy - vPosition) / vSize - vTexCoord - 0.5 / pixelsPerEm;

      vec4 p12 = (texture(uCurvesTexture, curveLoc / 512.0) - vec4(vPosition, vPosition)) / vec4(vSize, vSize) - vec4(vTexCoord, vTexCoord) - vec4(0.0, 0.5 / pixelsPerEm.y, 0.0, 0.5 / pixelsPerEm.y);
      vec2 p3 = (texture(uCurvesTexture, vec2(curveLoc.x + 1.0, curveLoc.y) / 512.0).xy - vPosition) / vSize - vTexCoord - vec2(0.0, 0.5 / pixelsPerEm.y);

      /* If the largest x coordinate among all three control points falls
       * left of the current pixel, then there are no more curves in the
       * horizontal band that can influence the result, so exit the loop.
       * (The curves are sorted in descending order by max x coordinate.)
       */

      if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) break;

      /* Generate the root contribution code based on the signs of the
       * y coordinates of the three control points.
       */

      uint code = (0x2E74U >> (((p12.y > 0.0) ? 2U : 0U) + ((p12.w > 0.0) ? 4U : 0U) + ((p3.y > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        // At least one root makes a contribution, so solve for the
        // values of t where the curve crosses y = 0. The quadratic
        // polynomial in t is given by
        //
        //     a t^2 - 2b t + c,
        //
        // where a = p1.y - 2 p2.y + p3.y, b = p1.y - p2.y, and c = p1.y.
        // The discriminant b^2 - ac is clamped to zero, and imaginary
        // roots are treated as a double root at the global minimum
        // where t = b / a.

        for (int sample_i = 0; sample_i < sampleCount.y; sample_i++) {
          p12 += vec4(0.0, float(sample_i + 1) / (float(sampleCount.y + 1) * pixelsPerEm.y), 0.0, float(sample_i + 1) / (float(sampleCount.y + 1) * pixelsPerEm.y));
          p3 += vec2(0.0, float(sample_i + 1) / (float(sampleCount.y + 1) * pixelsPerEm.y));

          float ax = p12.x - p12.z * 2.0 + p3.x;
          float ay = p12.y - p12.w * 2.0 + p3.y;
          float bx = p12.x - p12.z;
          float by = p12.y - p12.w;
          float ra = 1.0 / ay;

          float d = sqrt(max(by * by - ay * p12.y, 0.0));
          float t1 = (by - d) * ra;
          float t2 = (by + d) * ra;

          // If the polynomial is nearly linear, then solve -2b t + c = 0.

          if (abs(ay) < kQuadraticEpsilon) t1 = t2 = p12.y * 0.5 / by;

          // Calculate the x coordinates where C(t) = 0, and transform
          // them so that the current pixel corresponds to the range
          // [0,1]. Clamp the results and use them for root contributions.

          float x1 = (ax * t1 - bx * 2.0) * t1 + p12.x;
          float x2 = (ax * t2 - bx * 2.0) * t2 + p12.x;
          x1 = clamp(x1 * pixelsPerEm.x + 0.5, 0.0, 1.0);
          x2 = clamp(x2 * pixelsPerEm.x + 0.5, 0.0, 1.0);

          // Bits in code tell which roots make a contribution.

          if ((code & 1U) != 0U) coverage += x1 / float(sampleCount.y);
          if (code > 1U) coverage -= x2 / float(sampleCount.y);
        }
      }
    }

    // Loop over all curves in the vertical band.

    for (uint curve = 0U; curve < 2U; curve++) {
      // ivec2 curveLoc = ivec2(texelFetch(bandTex, ivec2(vbandLoc.x + curve, vbandLoc.y)).xy);
      vec2 curveLoc = vec2(float(curve), 0.0);

      // vec4 p12 = (texture(uCurvesTexture, curveLoc / 512.0) - vec4(vPosition, vPosition)) / vec4(vSize, vSize) - vec4(vTexCoord, vTexCoord);
      // vec2 p3 = (texture(uCurvesTexture, vec2(curveLoc.x + 1.0, curveLoc.y) / 512.0).xy - vPosition) / vSize - vTexCoord;

      vec4 p12 = (texture(uCurvesTexture, curveLoc / 512.0) - vec4(vPosition, vPosition)) / vec4(vSize, vSize) - vec4(vTexCoord, vTexCoord) - vec4(0.5 / pixelsPerEm.x, 0.0, 0.5 / pixelsPerEm.x, 0.0);
      vec2 p3 = (texture(uCurvesTexture, vec2(curveLoc.x + 1.0, curveLoc.y) / 512.0).xy - vPosition) / vSize - vTexCoord - vec2(0.5 / pixelsPerEm.x, 0.0);

      // If the largest y coordinate among all three control points falls
      // below the current pixel, then exit the loop.

      if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) break;

      // Generate the root contribution code based on the signs of the
      // x coordinates of the three control points.

      uint code = (0x2E74U >> (((p12.x > 0.0) ? 2U : 0U) +
              ((p12.z > 0.0) ? 4U : 0U) + ((p3.x > 0.0) ? 8U : 0U))) & 3U;

      if (code != 0U) {
        // At least one root makes a contribution, so solve for the
        // values of t where the rotated curve crosses y = 0.

        for (int sample_i = 0; sample_i < sampleCount.x; sample_i++) {
          p12 += vec4(float(sample_i + 1) / (float(sampleCount.x + 1) * pixelsPerEm.x), 0.0, float(sample_i + 1) / (float(sampleCount.x + 1) * pixelsPerEm.x), 0.0);
          p3 += vec2(float(sample_i + 1) / (float(sampleCount.x + 1) * pixelsPerEm.x), 0.0);

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

    coverage = clamp(abs(coverage) * 0.5, 0.0, 1.0);
    // coverage = sqrt(clamp(abs(coverage) * 0.5, 0.0, 1.0));
	  float alpha = coverage * vColor.a;
	  // oFragColor = vec4(float(sampleCount.x) / 8.0 * alpha, 0.0, 0.0, alpha);
	  oFragColor = vec4(vColor.rgb * alpha, alpha);
    // oFragColor = vColor * vec4(texture(uCurvesTexture, vec2(0.0, 0.0))) * vec4(pixelsPerEm.x, pixelsPerEm.y, 1.0, 1.0);
  }

)"
