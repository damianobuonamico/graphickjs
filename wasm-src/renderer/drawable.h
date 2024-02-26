#pragma once

#include "properties.h"
#include "geometry/contour.h"

#include "../math/rect.h"

namespace Graphick::Renderer {

  using Paint = Fill;

  struct Drawable {
    std::vector<Geometry::Contour> contours;
    Paint paint;
    f24x8x4 bounds;

    Drawable() = default;
    Drawable(const size_t i, const Paint& paint, const f24x8x4& bounds) : contours(i), paint(paint), bounds(bounds) {}
    Drawable(const size_t i, const Paint& paint, const rect& bounds) : contours(i), paint(paint) {
      this->bounds = {
        Math::float_to_f24x8(bounds.min.x),
        Math::float_to_f24x8(bounds.min.y),
        Math::float_to_f24x8(bounds.max.x),
        Math::float_to_f24x8(bounds.max.y)
      };
    }
    Drawable(const size_t i, Paint&& paint, const f24x8x4& bounds) : contours(i), paint(paint), bounds(bounds) {}
  };

  struct OutlineDrawable {
    std::vector<Geometry::OutlineContour> contours;

    OutlineDrawable() = default;
    OutlineDrawable(const size_t i) : contours(i) {}
  };

}