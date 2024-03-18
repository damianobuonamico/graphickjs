/**
 * @file vector.cpp
 * @brief Contains the implementation of the vector functions.
 */

#include "vector.h"

#include "scalar.h"

namespace Graphick::Math {

  /* -- cubic -- */

  std::vector<float> cubic_extrema(const vec2 v1, const vec2 v2, const vec2 v3, const vec2 v4) {
    const vec2 a = 3.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
    const vec2 b = 6.0f * (v1 - 2.0f * v2 + v3);
    const vec2 c = 3.0f * (v2 - v1);

    std::vector<float> roots = { 0.0f, 1.0f };

    for (int i = 0; i < 2; i++) {
      if (is_almost_zero(a[i])) {
        if (is_almost_zero(b[i])) continue;

        float t = -c[i] / b[i];
        if (t > 0.0f && t < 1.0f) {
          roots.push_back(t);
        }

        continue;
      }

      float delta = b[i] * b[i] - 4.0f * a[i] * c[i];

      if (is_almost_zero(delta)) {
        roots.push_back(-b[i] / (2.0f * a[i]));
      } else if (delta < 0.0f) {
        continue;
      } else {
        float sqrt_delta = std::sqrtf(delta);

        float t1 = (-b[i] + sqrt_delta) / (2.0f * a[i]);
        float t2 = (-b[i] - sqrt_delta) / (2.0f * a[i]);

        if (t1 > 0.0f && t1 < 1.0f) {
          roots.push_back(t1);
        }
        if (t2 > 0.0f && t2 < 1.0f) {
          roots.push_back(t2);
        }
      }
    }

    return roots;
  }

  /* -- angle -- */

  float angle(const vec2 v1, const vec2 v2) {
    return sign(v1.x * v2.y - v1.y * v2.x) * std::acos(dot(v1, v2) / (length(v1) * length(v2)));
  }

  /* -- collinear -- */
  bool collinear(const vec2 v1, const vec2 v2, const vec2 v3, const float eps) {
    float t = v1.x * (v2.y - v3.y) + v2.x * (v3.y - v1.y) + v3.x * (v1.y - v2.y);
    return is_almost_zero(t, eps);
  }

}
