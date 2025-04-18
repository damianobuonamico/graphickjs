/**
 * @file utils/color.h
 * @brief Contains the color conversion functions.
 */

#pragma once

#include "../math/scalar.h"
#include "../math/vec4.h"

namespace graphick::utils::color {

inline vec4 HSVA2RGBA(const vec4& hsva)
{
  const float h = hsva[0];
  const float s = hsva[1] / 100.0f;
  const float b = hsva[2] / 100.0f;

  auto k = [h](const float n) { return std::fmod(n + h / 60.0f, 6.0f); };
  auto f = [k, s, b](const float n) {
    return b * (1.0f - s * math::max(1.0f, math::min(k(n), 4.0f - k(n), 1.0f)));
  };

  return {math::clamp(f(5.0f), 0.0f, 1.0f),
          math::clamp(f(3.0f), 0.0f, 1.0f),
          math::clamp(f(1.0f), 0.0f, 1.0f),
          hsva[3]};
}

inline vec4 RGBA2HSVA(const vec4& rgba)
{
  const float v = math::max(rgba.r, rgba.g, rgba.b);
  const float n = v - math::min(rgba.r, rgba.g, rgba.b);
  const float h = math::is_almost_zero(n)          ? 0.0f :
                  math::is_almost_equal(v, rgba.r) ? (rgba.g - rgba.b) / n :
                  math::is_almost_equal(v, rgba.g) ? 2.0f + (rgba.b - rgba.r) / n :
                                                     4.0f + (rgba.r - rgba.g) / n;

  return {60.0f * (h < 0.0f ? h + 6.0f : h),
          math::is_almost_zero(v) ? 0.0f : (n / v) * 100.0f,
          v * 100.0f,
          rgba.a};
}

}  // namespace graphick::utils::color
