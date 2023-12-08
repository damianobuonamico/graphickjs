/**
 * @file path_builder.cpp
 * @brief PathBuilder class implementation
 *
 * @todo fix tolerance
 */

#include "path_builder.h"

#include "path_dev.h"

#include "../../math/math.h"

namespace Graphick::Renderer::Geometry {

  PathBuilder::PathBuilder(const rect& clip) : m_clip(clip) {}

  Drawable PathBuilder::fill(const PathDev& path, const Fill& fill) {
    Drawable drawable(0, fill, m_clip);
    Geometry::Contour* contour = nullptr;
    vec2 last;

    path.for_each(
      [&](const vec2 p0) {
        if (contour) contour->close();

        last = p0 - m_clip.min;

        contour = &drawable.contours.emplace_back();
        contour->move_to(f24x8x2{ Math::float_to_f24x8(last.x), Math::float_to_f24x8(last.y) });
      },
      [&](const vec2 p1) {
        contour->line_to(f24x8x2{ Math::float_to_f24x8(p1.x - m_clip.min.x), Math::float_to_f24x8(p1.y - m_clip.min.y) });
      },
      nullptr,
      [&](const vec2 p1, const vec2 p2, const vec2 p3) {
        rect bounds = { last, last };

        Math::min(bounds.min, p1, bounds.min);
        Math::min(bounds.min, p2, bounds.min);
        Math::min(bounds.min, p3, bounds.min);

        Math::max(bounds.max, p1, bounds.max);
        Math::max(bounds.max, p2, bounds.max);
        Math::max(bounds.max, p3, bounds.max);

        if (Math::does_rect_intersect_rect(bounds, m_clip)) {
          contour->cubic_to(
            f24x8x2{ Math::float_to_f24x8(p1.x - m_clip.min.x), Math::float_to_f24x8(p1.y - m_clip.min.y) },
            f24x8x2{ Math::float_to_f24x8(p2.x - m_clip.min.x), Math::float_to_f24x8(p2.y - m_clip.min.y) },
            f24x8x2{ Math::float_to_f24x8(p3.x - m_clip.min.x), Math::float_to_f24x8(p3.y - m_clip.min.y) }
          );
        } else {
          contour->line_to(f24x8x2{ Math::float_to_f24x8(p3.x - m_clip.min.x), Math::float_to_f24x8(p3.y - m_clip.min.y) });
        }
      }
    );

    return drawable;
  }

}
