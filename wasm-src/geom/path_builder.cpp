/**
 * @file geom/path_builder.h
 * @brief Containes the PathBuilder class definition and its options.
 */

#include "path_builder.h"

#include "cubic_bezier.h"
#include "cubic_path.h"
#include "curve_ops.h"
#include "geom.h"
#include "intersections.h"
#include "path.h"
#include "quadratic_bezier.h"
#include "quadratic_path.h"

#include "offset/offset.h"

#include "../math/math.h"
#include "../math/matrix.h"

#include "../utils/console.h"

namespace graphick::geom {

/* -- Static Methods -- */

/**
 * @brief Adds the given cap to the contour.
 *
 * @param from The start point of the cap.
 * @param to The end point of the cap.
 * @param n The normal of the cap.
 * @param radius The radius of the cap.
 * @param cap The cap type.
 * @param sink The contour to add the cap to.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void add_cap(const dvec2 from, const dvec2 to, const dvec2 n, const double radius, const LineCap cap, CubicPath<T>& sink) {
  switch (cap) {
  case LineCap::Round: {
    sink.arc_to(math::Vec2<T>(from + (to - from) / 2.0), math::Vec2<T>(to));
    break;
  }
  case LineCap::Square: {
    dvec2 dir = {-n.y * radius, n.x * radius};

    sink.line_to(math::Vec2<T>(from + dir));
    sink.line_to(math::Vec2<T>(to + dir));
    sink.line_to(math::Vec2<T>(to));

    break;
  }
  default:
  case LineCap::Butt: {
    sink.line_to(math::Vec2<T>(to));
    break;
  }
  }
}

/**
 * @brief Adds the given join to the contour.
 *
 * @param from The start point of the join.
 * @param to The end point of the join.
 * @param pivot The pivot point of the join.
 * @param from_normal The normal of the start point.
 * @param to_normal The normal of the end point.
 * @param radius The radius of the join.
 * @param inv_miter_limit The inverse miter limit.
 * @param join The join type.
 * @param sink The contour to add the join to.
 * @param reverse Whether to consider the complementary angle.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void add_join(
  const dvec2 from,
  const dvec2 to,
  const dvec2 pivot,
  const dvec2 from_normal,
  const dvec2 to_normal,
  const double radius,
  const double inv_miter_limit,
  LineJoin join,
  CubicPath<T>& sink,
  math::Rect<T>& bounding_rect,
  const bool small_segment,
  const bool reverse = false
) {
  if (math::is_almost_equal(from, to, math::geometric_epsilon<double>)) {
    return;
  }

  const dvec2 a = from - pivot;
  const dvec2 b = to - pivot;

  double dot = a.x * b.x + a.y * b.y;
  double cross = a.x * b.y - a.y * b.x;

  if (reverse) cross = -cross;

  double ang = std::atan2(cross, dot);
  bool concave = false;

  if (ang < 0.0) ang += math::two_pi<double>;
  if (ang >= math::pi<double>) {
    join = LineJoin::Bevel;
    concave = true;
  }

  if (math::is_almost_zero(ang)) {
    return;
  }

  switch (join) {
  case LineJoin::Round: {
    sink.arc_to(math::Vec2<T>(pivot), math::Vec2<T>(to), !reverse);
    break;
  }
  case LineJoin::Miter: {
    double dot = from_normal.x * to_normal.x + from_normal.y * to_normal.y;
    double sin_half = std::sqrt((1.0 + dot) * 0.5);

    if (sin_half < inv_miter_limit) {
      sink.line_to(math::Vec2<T>(to));
      break;
    } else {
      dvec2 mid = from_normal + to_normal;
      double l = radius / (sin_half * std::hypot(mid.x, mid.y));
      dvec2 p = pivot + mid * l;

      sink.line_to(math::Vec2<T>(p));
      sink.line_to(math::Vec2<T>(to));

      bounding_rect.include(math::Vec2<T>(p));

      break;
    }
  }
  default:
  case LineJoin::Bevel: {
    if (concave && small_segment) {
      sink.line_to(math::Vec2<T>(pivot));
    }

    sink.line_to(math::Vec2<T>(to));

    break;
  }
  }
}

/**
 * @brief Flattens a quadratic bezier curve and outputs the line segments to a sink vector.
 *
 * This method uses a fast flattening algorithm that will create lots of extra lines if the curve is too large.
 *
 * @tparam T The type of the output coordinates.
 * @param quad The quadratic bezier curve to flatten.
 * @param tolerance The tolerance to use when flattening the curve.
 * @param sink_callback The callback to output the lines to.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void fast_flatten(
  const dquadratic_bezier& quad,
  const double tolerance,
  std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
) {
  const auto& [a, b, c] = quad.coefficients();
  const double dt = std::sqrt((2.0 * tolerance) / math::length(quad.p0 - 2.0 * quad.p1 + quad.p2));

  dvec2 last = quad.p0;
  double t = dt;

  while (t < 1.0) {
    const double t_sq = t * t;
    const dvec2 p = a * t_sq + b * t + c;

    sink_callback(math::Vec2<T>(last), math::Vec2<T>(p));

    last = p;
    t += dt;
  }

  sink_callback(math::Vec2<T>(last), math::Vec2<T>(quad.p2));
}

/**
 * @brief Flattens a cubic bezier curve and outputs the line segments to a sink vector.
 *
 * This method uses a fast flattening algorithm that will create lots of extra lines if the curve is too large.
 *
 * @tparam T The type of the output coordinates.
 * @param cubic The cubic bezier curve to flatten.
 * @param tolerance The tolerance to use when flattening the curve.
 * @param sink_callback The callback to output the lines to.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void fast_flatten(
  const dcubic_bezier& cubic,
  const double tolerance,
  std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
) {
  const auto& [a, b, c, d] = cubic.coefficients();
  const double conc = std::max(std::hypot(b.x, b.y), std::hypot(a.x + b.x, a.y + b.y));
  const double dt = std::sqrt((std::sqrt(8.0) * tolerance) / conc);

  dvec2 last = cubic.p0;
  double t = dt;

  while (t < 1.0) {
    const double t_sq = t * t;
    const dvec2 p = a * t_sq * t + b * t_sq + c * t + d;

    sink_callback(math::Vec2<T>(last), math::Vec2<T>(p));

    last = p;
    t += dt;
  }

  sink_callback(math::Vec2<T>(last), math::Vec2<T>(cubic.p3));
}

/**
 * @brief Recursively flattens a quadratic bezier curve and outputs the line segments to a sink vector.
 *
 * This method uses a recursive flattening algorithm that will create less extra lines than the fast flattening algorithm.
 *
 * @tparam T The type of the output coordinates.
 * @param quad The quadratic bezier curve to flatten.
 * @param clip The rectangle to clip the curve to.
 * @param tolerance_sq The squared tolerance to use when flattening the curve.
 * @param sink_callback The callback to output the lines to.
 * @param depth The current recursion depth.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void recursive_flatten(
  const dquadratic_bezier& quad,
  const drect& clip,
  const double tolerance_sq,
  std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback,
  uint8_t depth = 0
) {
  if (depth > math::max_recursion_depth<uint8_t>) {
    sink_callback(math::Vec2<T>(quad.p0), math::Vec2<T>(quad.p2));
    return;
  }

  const drect bounds = quad.approx_bounding_rect();

  if (!does_rect_intersect_rect(bounds, clip)) {
    return;
  }

  depth += 1;

  const dvec2 p01 = (quad.p0 + quad.p1) * 0.5;
  const dvec2 p12 = (quad.p1 + quad.p2) * 0.5;
  const dvec2 p012 = (p01 + p12) * 0.5;

  const double den = math::squared_distance(quad.p0, quad.p2);
  const double num = std::abs((quad.p2.x - quad.p0.x) * (quad.p0.y - p012.y) - (quad.p0.x - p012.x) * (quad.p2.y - quad.p0.y));

  const double sq_error = num * num / den;

  if (sq_error < tolerance_sq) {
    sink_callback(math::Vec2<T>(quad.p0), math::Vec2<T>(quad.p2));
    return;
  }

  recursive_flatten(dquadratic_bezier{quad.p0, p01, p012}, clip, tolerance_sq, sink_callback, depth);
  recursive_flatten(dquadratic_bezier{p012, p12, quad.p2}, clip, tolerance_sq, sink_callback, depth);
}

/**
 * @brief Recursively flattens a cubic bezier curve and outputs the line segments to a sink vector.
 *
 * This method uses a recursive flattening algorithm that will create less extra lines than the fast flattening algorithm.
 *
 * @tparam T The type of the output coordinates.
 * @param cubic The cubic bezier curve to flatten.
 * @param clip The rectangle to clip the curve to.
 * @param tolerance_sq The squared tolerance to use when flattening the curve.
 * @param sink_callback The callback to output the lines to.
 * @param depth The current recursion depth.
 */
template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
static void recursive_flatten(
  const dcubic_bezier& cubic,
  const drect& clip,
  const double tolerance_sq,
  std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback,
  uint8_t depth = 0
) {
  if (depth > math::max_recursion_depth<uint8_t>) {
    sink_callback(math::Vec2<T>(cubic.p0), math::Vec2<T>(cubic.p3));
    return;
  }

  const drect bounds = cubic.approx_bounding_rect();

  if (!does_rect_intersect_rect(bounds, clip)) {
    return;
  }

  depth += 1;

  const dvec2 a = cubic.p3 - cubic.p0;
  const dvec2 b = cubic.p1 - cubic.p0;
  const dvec2 c = cubic.p2 - cubic.p0;

  const double num1 = math::cross(a, b);
  const double num2 = math::cross(a, c);
  const double one_over_den = 1.0 / math::squared_length(a);

  if (num1 * num1 * one_over_den < tolerance_sq && num2 * num2 * one_over_den < tolerance_sq) {
    sink_callback(math::Vec2<T>(cubic.p0), math::Vec2<T>(cubic.p3));
    return;
  }

  const auto& [left, right] = split(cubic, 0.5);

  recursive_flatten(left, clip, tolerance_sq, sink_callback, depth);
  recursive_flatten(right, clip, tolerance_sq, sink_callback, depth);
}

/* -- PathBuilder -- */

template <typename T, typename _>
PathBuilder<T, _>::PathBuilder(
  const QuadraticPath<T, std::enable_if<true>>& path,
  const math::Mat2x3<T>& transform,
  const math::Rect<T>* bounding_rect,
  const bool pretransformed_rect
) :
  m_quadratic_path(&path), m_transform(dmat2x3(transform)), m_type(PathType::Quadratic) {
  const drect bounds = bounding_rect ? drect(*bounding_rect) : drect(path.approx_bounding_rect());
  m_bounding_rect = pretransformed_rect ? bounds : m_transform * bounds;
}

template <typename T, typename _>
PathBuilder<T, _>::PathBuilder(
  const CubicPath<T>& path,
  const math::Mat2x3<T>& transform,
  const math::Rect<T>* bounding_rect,
  const bool pretransformed_rect
) :
  m_cubic_path(&path), m_transform(dmat2x3(transform)), m_type(PathType::Cubic) {
  const drect bounds = bounding_rect ? drect(*bounding_rect) : drect(path.approx_bounding_rect());
  m_bounding_rect = pretransformed_rect ? bounds : m_transform * bounds;
}

template <typename T, typename _>
PathBuilder<T, _>::PathBuilder(
  const Path<T, std::enable_if<true>>& path,
  const math::Mat2x3<T>& transform,
  const math::Rect<T>* bounding_rect,
  const bool pretransformed_rect
) :
  m_generic_path(&path), m_transform(dmat2x3(transform)), m_type(PathType::Generic) {
  const drect bounds = bounding_rect ? drect(*bounding_rect) : drect(path.approx_bounding_rect());
  m_bounding_rect = pretransformed_rect ? bounds : m_transform * bounds;
}

template <typename T, typename _>
void PathBuilder<T, _>::flatten(
  const math::Rect<T>& clip,
  const T tolerance,
  std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
) const {
  if (m_type != PathType::Generic || m_generic_path->empty()) return;

  const drect clipping_rect = drect(clip);
  const double coverage = rect_rect_intersection_area(m_bounding_rect, clipping_rect) / m_bounding_rect.area();

  if (coverage <= 0.0) {
    return;
  } else if (coverage <= 0.5) {
    flatten_clipped(clipping_rect, static_cast<double>(tolerance) * static_cast<double>(tolerance), sink_callback);
  } else {
    flatten_unclipped(static_cast<double>(tolerance), sink_callback);
  }
}

template <typename T, typename _>
StrokeOutline<T> PathBuilder<T, _>::stroke(const StrokingOptions<T>& options, const T tolerance) const {
  if (m_type != PathType::Generic || m_generic_path->empty()) return {};

  GK_TOTAL("PathBuilder::stroke()");

  const double radius = static_cast<double>(options.width) * 0.5;
  const double inv_miter_limit = 1.0 / static_cast<double>(options.miter_limit);

  dvec2 p0 = m_transform * dvec2(m_generic_path->at(0));

  StrokeOutline<T> outline;

  outline.bounding_rect = math::Rect<T>(m_bounding_rect);
  outline.bounding_rect.min -= radius;
  outline.bounding_rect.max += radius;

  if (m_generic_path->size() == 1 && m_generic_path->front().is_point()) {
    if (options.cap == LineCap::Butt) {
      return outline;
    }

    dvec2 n = {0.0, 1.0};
    dvec2 nr = n * radius;
    dvec2 start = p0 + nr;
    dvec2 rstart = p0 - nr;

    outline.inner.move_to(math::Vec2<T>(start));

    add_cap(start, rstart, n, radius, options.cap, outline.inner);
    add_cap(rstart, start, -n, radius, options.cap, outline.inner);

    return outline;
  }

  dvec2 start_n;

  if (math::is_almost_equal(m_generic_path->at(0), m_generic_path->at(1))) {
    if (math::is_almost_equal(m_generic_path->at(0), m_generic_path->at(2))) {
      start_n = math::normal(dvec2(m_generic_path->at(0)), dvec2(m_generic_path->at(3)));
    } else {
      start_n = math::normal(dvec2(m_generic_path->at(0)), dvec2(m_generic_path->at(2)));
    }
  } else {
    start_n = math::normal(dvec2(m_generic_path->at(0)), dvec2(m_generic_path->at(1)));
  }

  dvec2 last_n = start_n;

  if (m_generic_path->closed()) {
    outline.inner.move_to(math::Vec2<T>(p0 - last_n * radius));
    outline.outer.move_to(math::Vec2<T>(p0 + last_n * radius));
  } else {
    const dvec2 start = p0 - last_n * radius;

    outline.inner.move_to(math::Vec2<T>(start));
    outline.outer.move_to(math::Vec2<T>(start));

    add_cap(start, p0 + last_n * radius, -last_n, radius, options.cap, outline.outer);
  }

  m_generic_path->for_each(
    nullptr,
    [&](const math::Vec2<T> p1) {
      const dline line = {p0, m_transform * dvec2(p1)};

      const dvec2 start_n = math::normal(line.p0, line.p1);
      const dvec2 start_nr = start_n * radius;
      const dvec2 inner_start = line.p0 - start_nr;
      const dvec2 outer_start = line.p0 + start_nr;

      const bool small_segment = math::squared_distance(line.p0, line.p1) < radius * radius;

      add_join(
        dvec2(outline.inner.back()),
        inner_start,
        p0,
        -last_n,
        -start_n,
        radius,
        inv_miter_limit,
        options.join,
        outline.inner,
        outline.bounding_rect,
        small_segment,
        true
      );
      add_join(
        dvec2(outline.outer.back()),
        outer_start,
        p0,
        last_n,
        start_n,
        radius,
        inv_miter_limit,
        options.join,
        outline.outer,
        outline.bounding_rect,
        small_segment
      );

      outline.inner.line_to(math::Vec2<T>(line.p1 - start_nr));
      outline.outer.line_to(math::Vec2<T>(line.p1 + start_nr));

      last_n = start_n;
      p0 = line.p1;
    },
    nullptr,
    [&](const math::Vec2<T> p1, const math::Vec2<T> p2, const math::Vec2<T> p3) {
      const dcubic_bezier cubic = {p0, m_transform * dvec2(p1), m_transform * dvec2(p2), m_transform * dvec2(p3)};

      const dvec2 start_n = cubic.start_normal();
      const dvec2 start_nr = start_n * radius;

      const dvec2 inner_start = cubic.p0 - start_nr;
      const dvec2 outer_start = cubic.p0 + start_nr;

      add_join(
        dvec2(outline.inner.back()),
        inner_start,
        p0,
        -last_n,
        -start_n,
        radius,
        inv_miter_limit,
        options.join,
        outline.inner,
        outline.bounding_rect,
        false,
        true
      );
      add_join(
        dvec2(outline.outer.back()),
        outer_start,
        p0,
        last_n,
        start_n,
        radius,
        inv_miter_limit,
        options.join,
        outline.outer,
        outline.bounding_rect,
        false
      );

      // TODO: maybe join the two in one function call
      offset_cubic(cubic, -radius, tolerance, outline.inner);
      offset_cubic(cubic, radius, tolerance, outline.outer);

      last_n = cubic.end_normal();
      p0 = cubic.p3;
    }
  );

  if (m_generic_path->closed()) {
    add_join(
      dvec2(outline.inner.back()),
      dvec2(outline.inner.front()),
      p0,
      -last_n,
      -start_n,
      radius,
      inv_miter_limit,
      options.join,
      outline.inner,
      outline.bounding_rect,
      false,
      true
    );
    add_join(
      dvec2(outline.outer.back()),
      dvec2(outline.outer.front()),
      p0,
      last_n,
      start_n,
      radius,
      inv_miter_limit,
      options.join,
      outline.outer,
      outline.bounding_rect,
      false
    );

    std::reverse(outline.inner.points.begin(), outline.inner.points.end());
  } else {
    add_cap(dvec2(outline.outer.points.back()), dvec2(outline.inner.points.back()), last_n, radius, options.cap, outline.outer);

    outline.outer.points.insert(outline.outer.points.end(), outline.inner.points.rbegin() + 1, outline.inner.points.rend());
    outline.inner.points.clear();
  }

  return outline;
}

template <typename T, typename _>
void PathBuilder<T, _>::flatten_clipped(
  const drect& clip,
  const double tolerance_sq,
  std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
) const {
  dvec2 p0;

  m_generic_path->for_each(
    [&](const math::Vec2<T> p0_raw) { p0 = m_transform * dvec2(p0_raw); },
    [&](const math::Vec2<T> p1_raw) {
      const dvec2 p1 = m_transform * dvec2(p1_raw);

      sink_callback(math::Vec2<T>(p0), math::Vec2<T>(p1));

      p0 = p1;
    },
    [&](const math::Vec2<T> p1_raw, const math::Vec2<T> p2_raw) {
      const dvec2 p1 = m_transform * dvec2(p1_raw);
      const dvec2 p2 = m_transform * dvec2(p2_raw);

      recursive_flatten(dquadratic_bezier{p0, p1, p2}, clip, tolerance_sq, sink_callback);

      p0 = p2;
    },
    [&](const math::Vec2<T> p1_raw, const math::Vec2<T> p2_raw, const math::Vec2<T> p3_raw) {
      const dvec2 p1 = m_transform * dvec2(p1_raw);
      const dvec2 p2 = m_transform * dvec2(p2_raw);
      const dvec2 p3 = m_transform * dvec2(p3_raw);

      recursive_flatten(dcubic_bezier{p0, p1, p2, p3}, clip, tolerance_sq, sink_callback);

      p0 = p3;
    }
  );
}

template <typename T, typename _>
void PathBuilder<T, _>::flatten_unclipped(
  const double tolerance,
  std::function<void(const math::Vec2<T>, const math::Vec2<T>)> sink_callback
) const {

  dvec2 p0;
  m_generic_path->for_each(
    [&](const math::Vec2<T> p0_raw) { p0 = m_transform * dvec2(p0_raw); },
    [&](const math::Vec2<T> p1_raw) {
      const dvec2 p1 = m_transform * dvec2(p1_raw);

      sink_callback(math::Vec2<T>(p0), math::Vec2<T>(p1));

      p0 = p1;
    },
    [&](const math::Vec2<T> p1_raw, const math::Vec2<T> p2_raw) {
      const dvec2 p1 = m_transform * dvec2(p1_raw);
      const dvec2 p2 = m_transform * dvec2(p2_raw);

      fast_flatten(dquadratic_bezier{p0, p1, p2}, tolerance, sink_callback);

      p0 = p2;
    },
    [&](const math::Vec2<T> p1_raw, const math::Vec2<T> p2_raw, const math::Vec2<T> p3_raw) {
      const dvec2 p1 = m_transform * dvec2(p1_raw);
      const dvec2 p2 = m_transform * dvec2(p2_raw);
      const dvec2 p3 = m_transform * dvec2(p3_raw);

      fast_flatten(dcubic_bezier{p0, p1, p2, p3}, tolerance, sink_callback);

      p0 = p3;
    }
  );
}

/* -- Template instantiations -- */

template class PathBuilder<float>;
template class PathBuilder<double>;
}  // namespace graphick::geom
