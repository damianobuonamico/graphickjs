/**
 * @file path_builder.cpp
 * @brief PathBuilder class implementation
 *
 * @todo fix tiger whiskers
 * @todo fix rect intersection transforming points and then checking
 * @todo outline visible intersection for optimizing tesselation
 * @todo stroke and fill intersection for optimizing tesselation
 */

#include "path_builder.h"

#include "path.h"

#include "../../math/mat2x3.h"
#include "../../math/vector.h"
#include "../../math/math.h"

namespace Graphick::Renderer::Geometry {

  static constexpr double normal_eps = 0.5;    /* The epsilon to use for normal computation. */

  /**
   * @brief Computes the normals at the start and end of a cubic bezier curve.
   *
   * @param a The first point of the curve.
   * @param b The second point of the curve.
   * @param c The third point of the curve.
   * @param d The fourth point of the curve.
   * @return A pair of normals.
   */
  static std::pair<dvec2, dvec2> cubic_normals(const dvec2 a, const dvec2 b, const dvec2 c, const dvec2 d) {
    dvec2 normal_ab;
    dvec2 normal_bc;
    dvec2 normal_cd;

    if (Math::is_almost_equal(a, b, normal_eps)) {
      if (Math::is_almost_equal(a, c, normal_eps)) normal_ab = Math::normal(a, d);
      else normal_ab = Math::normal(a, c);
    } else normal_ab = Math::normal(a, b);

    if (Math::is_almost_equal(b, c, normal_eps)) {
      if (Math::is_almost_equal(b, d, normal_eps)) normal_bc = Math::normal(a, d);
      else normal_bc = Math::normal(b, d);
    } else normal_bc = Math::normal(b, c);

    if (Math::is_almost_equal(c, d, normal_eps)) {
      if (Math::is_almost_equal(b, d, normal_eps)) normal_cd = Math::normal(a, d);
      else normal_cd = Math::normal(b, d);
    } else normal_cd = Math::normal(c, d);

    return { normal_ab, normal_cd };
  }

  PathBuilder::PathBuilder(const drect& clip, const dmat2x3& transform, const double tolerance) :
    m_clip(clip), m_transform(transform), m_tolerance(tolerance) {}

  OutlineDrawable PathBuilder::outline(const Path& path) const {
    OutlineDrawable drawable{ 0 };
    OutlineContour* contour = nullptr;

    dvec2 last;

    path.for_each(
      [&](const vec2 p0) {
        dvec2 a = m_transform * dvec2(p0);

        if (contour) contour->close();

        contour = &drawable.contours.emplace_back(m_tolerance);
        contour->move_to(a);

        last = a;
      },
      [&](const vec2 p1) {
        dvec2 b = m_transform * dvec2(p1);

        contour->line_to(b);

        last = b;
      },
      nullptr,
      [&](const vec2 p1, const vec2 p2, const vec2 p3) {
        dvec2 b = m_transform * dvec2(p1);
        dvec2 c = m_transform * dvec2(p2);
        dvec2 d = m_transform * dvec2(p3);

        drect bounds = { last };

        Math::min(bounds.min, b, bounds.min);
        Math::min(bounds.min, c, bounds.min);
        Math::min(bounds.min, d, bounds.min);

        Math::max(bounds.max, b, bounds.max);
        Math::max(bounds.max, c, bounds.max);
        Math::max(bounds.max, d, bounds.max);

        if (Math::does_rect_intersect_rect(bounds, m_clip)) {
          contour->cubic_to(b, c, d);
        } else {
          contour->line_to(d);
        }

        last = d;
      }
    );

    if (contour && path.closed()) contour->close();

    return drawable;
  }

  Drawable PathBuilder::fill(const Path& path, const Fill& fill) const {
    Drawable drawable{ 0, fill, m_clip };
    Contour* contour = nullptr;

    dvec2 last;

    path.for_each(
      [&](const vec2 p0) {
        dvec2 a = m_transform * dvec2(p0);

        if (contour) contour->close();

        contour = &drawable.contours.emplace_back(m_tolerance);
        contour->move_to(a);

        last = a;
      },
      [&](const vec2 p1) {
        dvec2 b = m_transform * dvec2(p1);

        contour->line_to(b);

        last = b;
      },
      nullptr,
      [&](const vec2 p1, const vec2 p2, const vec2 p3) {
        dvec2 b = m_transform * dvec2(p1);
        dvec2 c = m_transform * dvec2(p2);
        dvec2 d = m_transform * dvec2(p3);

        drect bounds = { last };

        Math::min(bounds.min, b, bounds.min);
        Math::min(bounds.min, c, bounds.min);
        Math::min(bounds.min, d, bounds.min);

        Math::max(bounds.max, b, bounds.max);
        Math::max(bounds.max, c, bounds.max);
        Math::max(bounds.max, d, bounds.max);

        if (Math::does_rect_intersect_rect(bounds, m_clip)) {
          contour->cubic_to(b, c, d);
        } else {
          contour->line_to(d);
        }

        last = d;
      }
    );

    if (contour) contour->close();

    return drawable;
  }

  Drawable PathBuilder::stroke(const Path& path, const Stroke& stroke) const {
    Drawable drawable(0, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, m_clip);
    Contour* contour = nullptr;

    const auto first_segment = path.front();
    const double radius = stroke.width / 2.0;
    const double inv_miter_limit = 1.0 / stroke.miter_limit;

    if (path.size() == 1 && first_segment.is_point() && stroke.cap != LineCap::Butt) {
      contour = &drawable.contours.emplace_back(m_tolerance);

      dvec2 from = m_transform * dvec2(first_segment.p0);
      dvec2 n = { 0.0, 1.0 };
      dvec2 nr = n * radius;
      dvec2 start = from + nr;
      dvec2 rstart = from - nr;

      contour->move_to(start);

      contour->add_cap(start, rstart, n, radius, stroke.cap);
      contour->add_cap(rstart, start, -n, radius, stroke.cap);

      return drawable;
    }

    dvec2 last_dir = { 0.0, 0.0 };
    dvec2 first_point = { 0.0, 0.0 };
    dvec2 last_point = { 0.0, 0.0 };
    dvec2 pivot = { 0.0, 0.0 };

    std::vector<bool> closed;
    std::vector<dvec2> last_points;
    std::vector<dvec2> first_points;
    std::vector<dvec2> last_dirs;

    bool is_first = true;

    /* Forward for the outer stroke. */

    path.for_each(
      [&](const vec2 p0) {
        closed.push_back(path.closed());
        last_points.push_back(last_point);
        first_points.push_back(first_point);
        last_dirs.push_back(last_dir);

        contour = &drawable.contours.emplace_back(m_tolerance);
        is_first = !closed.back();

        if (closed.back()) {
          const auto segment = path.back();

          switch (segment.type) {
          case Path::Command::Line: {
            const dvec2 a = m_transform * dvec2(segment.p0);
            const dvec2 b = m_transform * dvec2(segment.p1);

            const dvec2 n = Math::normal(a, b);

            pivot = b;
            last_dir = n;
            first_point = last_point = b + n * radius;

            break;
          }
          case Path::Command::Quadratic:
            break;
          case Path::Command::Cubic: {
            const dvec2 a = m_transform * dvec2(segment.p0);
            const dvec2 b = m_transform * dvec2(segment.p1);
            const dvec2 c = m_transform * dvec2(segment.p2);
            const dvec2 d = m_transform * dvec2(segment.p3);

            const auto [_, n] = cubic_normals(a, b, c, d);

            pivot = d;
            last_dir = n;
            first_point = last_point = d + n * radius;

            break;
          }
          default:
            break;
          }

          contour->move_to(last_point);
        } else {
          pivot = m_transform * dvec2(p0);
        }
      },
      [&](const vec2 p1) {
        const dvec2 a = pivot;
        const dvec2 b = m_transform * dvec2(p1);

        if (Math::is_almost_equal(a, b, (double)GK_POINT_EPSILON)) return;

        const dvec2 n = Math::normal(a, b);
        const dvec2 nr = n * radius;

        const dvec2 start = a + nr;

        if (is_first) {
          contour->move_to(start);
          first_point = start;
          is_first = false;
        } else {
          contour->add_join(last_point, start, pivot, last_dir, n, radius, inv_miter_limit, stroke.join);
        }

        last_dir = n;
        pivot = b;
        last_point = b + nr;

        contour->line_to(last_point);
      },
      nullptr,
      [&](const vec2 p1, const vec2 p2, const vec2 p3) {
        const dvec2 a = pivot;
        const dvec2 b = m_transform * dvec2(p1);
        const dvec2 c = m_transform * dvec2(p2);
        const dvec2 d = m_transform * dvec2(p3);

        if (
          Math::is_almost_equal(a, b, (double)GK_POINT_EPSILON) &&
          Math::is_almost_equal(a, c, (double)GK_POINT_EPSILON) &&
          Math::is_almost_equal(a, d, (double)GK_POINT_EPSILON)
          ) return;

        const auto [start_n, end_n] = cubic_normals(a, b, c, d);

        const dvec2 start = a + start_n * radius;

        if (is_first) {
          contour->move_to(start);
          first_point = start;
          is_first = false;
        } else {
          contour->add_join(last_point, start, pivot, last_dir, start_n, radius, inv_miter_limit, stroke.join);
        }

        last_dir = end_n;
        pivot = d;
        last_point = d + end_n * radius;

        contour->offset_cubic(a, b, c, d, last_dir, radius);
      }
    );

    last_points.push_back(last_point);
    first_points.push_back(first_point);
    last_dirs.push_back(last_dir);

    is_first = true;

    /* Backward for the inner stroke. */

    const auto handle_first = [&](const dvec2 start) {
      if (closed.back()) {
        const auto segment = path.front();

        dvec2 n1;
        dvec2 end1;

        switch (segment.type) {
        case Path::Command::Line: {
          const dvec2 a1 = m_transform * dvec2(segment.p1);
          const dvec2 b1 = m_transform * dvec2(segment.p0);

          n1 = Math::normal(a1, b1);
          end1 = b1;

          break;
        }
        case Path::Command::Quadratic:
          break;
        case Path::Command::Cubic: {
          const dvec2 a1 = m_transform * dvec2(segment.p3);
          const dvec2 b1 = m_transform * dvec2(segment.p2);
          const dvec2 c1 = m_transform * dvec2(segment.p1);
          const dvec2 d1 = m_transform * dvec2(segment.p0);

          n1 = cubic_normals(a1, b1, c1, d1).second;
          end1 = d1;

          break;
        }
        case Path::Command::Move: {
          n1 = last_dirs.back();
          end1 = last_points.back();
          break;
        }
        }

        last_point = end1 + n1 * radius;
        last_dir = n1;
        pivot = end1;

        contour->line_to(last_point);
        contour->add_join(last_point, start, pivot, last_dir, n1, radius, inv_miter_limit, stroke.join);
      } else {
        contour->add_cap(last_points.back(), start, last_dirs.back(), radius, stroke.cap);
      }

      last_points.pop_back();
      last_dirs.pop_back();

      is_first = false;
      };

    path.for_each_reversed(
      [&](const vec2 p0) {
        if (!closed.back()) {
          contour->add_cap(last_point, first_points.back(), last_dir, radius, stroke.cap);
        }

        contour->close();
        contour = &drawable.contours[0];

        first_points.pop_back();
        closed.pop_back();

        is_first = true;
      },
      [&](const vec2 p0, const vec2 p1) {
        if (Math::is_almost_equal(p0, p1, GK_POINT_EPSILON)) return;

        const dvec2 a = m_transform * dvec2(p1);
        const dvec2 b = m_transform * dvec2(p0);

        const dvec2 n = Math::normal(a, b);
        const dvec2 nr = n * radius;

        const dvec2 start = a + nr;

        if (is_first) {
          handle_first(start);
        } else {
          contour->add_join(last_point, start, pivot, last_dir, n, radius, inv_miter_limit, stroke.join);
        }

        last_dir = n;
        pivot = b;
        last_point = b + nr;

        contour->line_to(last_point);
      },
      nullptr,
      [&](const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3) {
        if (
          Math::is_almost_equal(p0, p1, GK_POINT_EPSILON) &&
          Math::is_almost_equal(p0, p3, GK_POINT_EPSILON) &&
          Math::is_almost_equal(p0, p3, GK_POINT_EPSILON)
          ) return;

        const dvec2 a = m_transform * dvec2(p3);
        const dvec2 b = m_transform * dvec2(p2);
        const dvec2 c = m_transform * dvec2(p1);
        const dvec2 d = m_transform * dvec2(p0);

        const auto [start_n, end_n] = cubic_normals(a, b, c, d);

        const dvec2 start = a + start_n * radius;

        if (is_first) {
          handle_first(start);
        } else {
          contour->add_join(last_point, start, pivot, last_dir, start_n, radius, inv_miter_limit, stroke.join);
        }

        last_dir = end_n;
        pivot = d;
        last_point = d + end_n * radius;

        contour->offset_cubic(a, b, c, d, last_dir, radius);
      }
    );

    return drawable;
  }

}
