#pragma once

#include "properties.h"
#include "geometry/contour.h"

namespace Graphick::Renderer {

  using Paint = Fill;

  struct Drawable {
    std::vector<Geometry::Contour> contours;
    Paint paint;
    f24x8x4 bounds;

    Drawable() = default;
    Drawable(const size_t i, const Paint& paint, const f24x8x4& bounds) : contours(i), paint(paint), bounds(bounds) {}
    Drawable(const size_t i, Paint&& paint, const f24x8x4& bounds) : contours(i), paint(paint), bounds(bounds) {}
  };

}