#pragma once

#include "properties.h"
#include "geometry/contour.h"

#include "../math/rect.h"

namespace Graphick::Renderer {

  using Paint = Fill;

  struct Drawable {
    std::vector<Geometry::Contour> contours;
    Paint paint;
    rect bounds;

    Drawable() = default;
    Drawable(const size_t i, const Paint& paint, const rect& bounds) : contours(i), paint(paint), bounds(bounds) {}
    Drawable(const size_t i, Paint&& paint, const rect& bounds) : contours(i), paint(paint), bounds(bounds) {}
  };

}