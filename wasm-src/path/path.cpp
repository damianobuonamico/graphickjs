/**
 * @file path.cpp
 * @brief Implementation of the Path class.
 *
 * @todo implement compound paths when groups are a thing.
 */

#include "path.h"
// #include "path_builder.h"
// #include "contour.h"

#include "../renderer/properties.h"

// #include "../algorithms/fit.h"
#include "../math/matrix.h"
#include "../math/math.h"

#include "../geom/intersections.h"
#include "../geom/curve_ops.h"

#include "../utils/console.h"
#include "../utils/assert.h"

namespace graphick::path {

  /* -- Static -- */

  /**
   * @brief Approximates a cubic bezier segment with a single quadratic bezier segment.
   *
   * This is terrible as a general approximation but works if the cubic curve does not have inflection points and is "flat" enough.
   * Typically usables after subdiving the curve a few times.
   *
   * @param cubic The cubic bezier segment to approximate.
   * @return The p1 control point of the quadratic bezier curve, p0 and p2 are the start and end points of the cubic curve.
   */
  static vec2 single_quadratic_approximation(const cubic_bezier& cubic) {
    if (math::is_almost_equal(cubic.p0, cubic.p1)) {
      return cubic.p2;
    } else if (math::is_almost_equal(cubic.p2, cubic.p3)) {
      return cubic.p1;
    }

    // const vec2 ab = cubic.p1 - cubic.p0;
    // const vec2 cd = cubic.p3 - cubic.p2;
    // const float pcd = math::cross(ab, cd);

    dvec2 p0 = dvec2(cubic.p0);
    dvec2 p1 = dvec2(cubic.p1);
    dvec2 p2 = dvec2(cubic.p2);
    dvec2 p3 = dvec2(cubic.p3);

    double d = (p0.x - p1.x) * (p2.y - p3.y) - (p0.y - p1.y) * (p2.x - p3.x);

    if (math::is_almost_zero(d, 0.000001)) {
    // if (math::is_almost_zero(pcd)) {
      // return std::nullopt;
      const dvec2 p1 = (p1 * 3.0 - p0) * 0.5;
      const dvec2 p2 = (p2 * 3.0 - p3) * 0.5;

      return vec2((p1 + p2) * 0.5);
    }

    double pre = (p0.x * p1.y - p0.y * p1.x), post = (p2.x * p3.y - p2.y * p3.x);

    double x = (pre * (p2.x - p3.x) - (p0.x - p1.x) * post) / d;
    double y = (pre * (p2.y - p3.y) - (p0.y - p1.y) * post) / d;

    return vec2(dvec2{ x, y });

    // const float h = math::cross(ab, (cubic.p0 - cubic.p2) / pcd);
    // return cubic.p2 + cd * h;
  }

  /**
   * @brief Evaluates an upper bound on the maximum distance between the cubic and its quadratic approximation.
   *
   * The approximation is obtained using the single_quadratic_approximation() method.
   * See http://caffeineowl.com/graphics/2d/vectorial/cubic2quad01.html
   *
   * @param cubic The cubic bezier segment to evaluate.
   * @return The maximum distance between the cubic and its quadratic approximation.
   */
  static float single_quadratic_approximation_error(const cubic_bezier& cubic) {
    return std::sqrtf(3.0f) / 36.0f * math::length((cubic.p3 - cubic.p2 * 3.0f) + (cubic.p1 * 3.0f - cubic.p0));
  }

  static void degenerate_cubic_to_quadratics(const cubic_bezier& cubic, const float tolerance, QuadraticPath& sink) {
    // Path::Segment sub_curve = cubic;

    // float t_min = 0.0f;
    // float t_max = 1.0f;

    // while (true) {
    //   if (single_quadratic_approximation_error(sub_curve) <= tolerance) {
    //     sink.quadratic_to(single_quadratic_approximation(sub_curve), sub_curve.p3);

    //     if (t_max >= 1.0f) {
    //       return;
    //     }

    //     t_min = t_max;
    //     t_max = 1.0f;
    //   } else {
    //     t_max = (t_min + t_max) / 2.0f;
    //   }

    //   const auto& [p0, p1, p2, p3] = math::split_bezier(cubic.p0, cubic.p1, cubic.p2, cubic.p3, t_min, t_max);

    //   sub_curve = { p0, p1, p2, p3 };
    // }
  }

  /**
   * @brief Approximates a monotonic cubic bezier curve with a sequence of quadratic bezier segments.
   *
   * @param cubic The cubic bezier segment to approximate.
   * @param tolerance The maximum error tolerance.
   * @param sink The quadratic path to append the approximated segments to.
   */
  static void monotonic_cubic_to_quadratics(const cubic_bezier& cubic, const float tolerance, QuadraticPath& sink) {
    // degenerate_cubic_to_quadratics(cubic, tolerance, sink);
  // if (math::is_almost_equal(cubic.p2, cubic.p3)) {
    // return;
  // }

    cubic_bezier sub_curve = cubic;

    float t_min = 0.0f;
    float t_max = 1.0f;

    while (true) {
      if (single_quadratic_approximation_error(sub_curve) <= tolerance) {
        sink.quadratic_to(single_quadratic_approximation(sub_curve), sub_curve.p3);

        if (t_max >= 1.0f) {
          return;
        }

        t_min = t_max;
        t_max = 1.0f;
      } else {
        t_max = (t_min + t_max) / 2.0f;
      }

      sub_curve = geom::extract(cubic, t_min, t_max);
    }
  }

  /* -- Segment -- */

  bool Path::Segment::is_point() const {
    bool point = p0 == p1;

    if (point) {
      if (is_quadratic()) return p1 == p2;
      if (is_cubic()) return p1 == p2 && p2 == p3;
    }

    return point;
  }

  vec2 Path::Segment::sample(const float t) const {
    switch (type) {
    case Command::Cubic:
      return geom::cubic({ p0, p1, p2, p3 }, t);
    case Command::Quadratic:
      return geom::quadratic({ p0, p1, p2 }, t);
    case Command::Line:
      return math::lerp(p0, p1, t);
    default:
    case Command::Move:
      return p0;
    }
  }

  math::rect Path::Segment::bounding_rect() const {
    switch (type) {
    case Command::Cubic: {
      return geom::bounding_rect(cubic_bezier{ p0, p1, p2, p3 });
    }
    case Command::Quadratic: {
      return geom::bounding_rect(quadratic_bezier{ p0, p1, p2 });
    }
    case Command::Line: {
      return math::rect::from_vectors(p0, p1);
    }
    default:
    case Command::Move: {
      return { p0, p0 };
    }
    }
  }

  math::rect Path::Segment::bounding_rect(const mat2x3& transform) const {
    const vec2 a = transform * p0;
    const vec2 b = transform * p1;

    switch (type) {
    case Command::Cubic: {
      const vec2 c = transform * p2;
      const vec2 d = transform * p3;

      return geom::bounding_rect(cubic_bezier{ a, b, c, d });
    }
    case Command::Quadratic: {
      const vec2 c = transform * p2;

      return geom::bounding_rect(quadratic_bezier{ a, b, c });
    }
    case Command::Line: {
      return math::rect::from_vectors(a, b);
    }
    default:
      return { a, a };
    }
  }

  math::rect Path::Segment::approx_bounding_rect() const {
    switch (type) {
    case Command::Cubic: {
      return geom::approx_bounding_rect(cubic_bezier{ p0, p1, p2, p3 });
    }
    case Command::Quadratic: {
      return geom::approx_bounding_rect(quadratic_bezier{ p0, p1, p2 });
    }
    case Command::Line: {
      return math::rect::from_vectors(p0, p1);
    }
    default:
      return {};
    }
  }

  /* -- Iterator -- */

  Path::Iterator::Iterator(const Path& path, const size_t index, const IndexType index_type) : m_path(path), m_index(index), m_point_index(0) {
    if (index_type == IndexType::Point) {
      GK_ASSERT(index < path.m_points.size(), "Point index out of range.");

      m_index = 0;

      while (m_point_index <= index) {
        switch (path.get_command(m_index)) {
        case Command::Move:
          if (index == m_point_index) {
            operator++();
            return;
          }

          m_point_index += 1;
          break;
        case Command::Line:
          if (index == m_point_index) {
            return;
          }

          m_point_index += 1;
          break;
        case Command::Quadratic:
          if (index - m_point_index <= 1) {
            return;
          }

          m_point_index += 2;
          break;
        case Command::Cubic:
          if (index - m_point_index <= 2) {
            return;
          }

          m_point_index += 3;
          break;
        }

        m_index += 1;
      }

      return;
    } else if (index_type == IndexType::Segment) {
      GK_ASSERT(index < path.size(), "Segment index out of range.");

      m_index = index + 1;
    }

    if (m_index < path.m_commands_size && path.get_command(m_index) == Command::Move) {
      m_index += 1;
    }

    GK_ASSERT(m_index > 0 && m_index <= path.m_commands_size, "Index out of range.");

    if (m_index <= path.m_commands_size / 2) {
      for (size_t i = 0; i < m_index; i++) {
        switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index += 1;
          break;
        case Command::Quadratic:
          m_point_index += 2;
          break;
        case Command::Cubic:
          m_point_index += 3;
          break;
        }
      }
    } else {
      m_point_index = path.m_points.size();

      for (size_t i = path.m_commands_size - 1; i >= m_index; i--) {
        switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index -= 1;
          break;
        case Command::Quadratic:
          m_point_index -= 2;
          break;
        case Command::Cubic:
          m_point_index -= 3;
          break;
        }
      }
    }
  }

  Path::Iterator& Path::Iterator::operator++() {
    GK_ASSERT(m_index < m_path.m_commands_size, "Cannot increment the end iterator.");

    switch (m_path.get_command(m_index)) {
    case Command::Move:
    case Command::Line:
      m_point_index += 1;
      break;
    case Command::Quadratic:
      m_point_index += 2;
      break;
    case Command::Cubic:
      m_point_index += 3;
      break;
    }

    m_index += 1;

    if (m_index < m_path.m_commands_size && m_path.get_command(m_index) == Command::Move) operator++();

    return *this;
  }

  Path::Iterator Path::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++(*this);

    return tmp;
  }

  Path::Iterator Path::Iterator::operator+(const size_t n) const {
    Iterator tmp = *this;

    for (size_t i = 0; i < n; i++) {
      ++tmp;
    }

    return tmp;
  }

  Path::Iterator& Path::Iterator::operator--() {
    GK_ASSERT(m_index > 0, "Cannot decrement the begin iterator.");

    m_index -= 1;

    switch (m_path.get_command(m_index)) {
    case Command::Move:
      operator--();
    case Command::Line:
      m_point_index -= 1;
      break;
    case Command::Quadratic:
      m_point_index -= 2;
      break;
    case Command::Cubic:
      m_point_index -= 3;
      break;
    }

    return *this;
  }

  Path::Iterator Path::Iterator::operator--(int) {
    Iterator tmp = *this;
    --(*this);

    return tmp;
  }

  Path::Iterator Path::Iterator::operator-(const size_t n) const {
    Iterator tmp = *this;

    for (size_t i = 0; i < n; i++) {
      --tmp;
    }

    return tmp;
  }

  Path::Iterator::value_type Path::Iterator::operator*() const {
    const Command command = m_path.get_command(m_index);

    switch (command) {
    case Command::Cubic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 2 < m_path.m_points.size(), "Not enough points for a cubic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1], m_path.m_points[m_point_index + 2] };
    }
    case Command::Quadratic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 1 < m_path.m_points.size(), "Not enough points for a quadratic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1] };
    }
    case Command::Line: {
      GK_ASSERT(m_point_index > 0 && m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index] };
    }
    default:
    case Command::Move:
      GK_ASSERT(m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index] };
    }
  }

  /* -- ReverseIterator -- */

  Path::ReverseIterator::ReverseIterator(const Path& path, const size_t index) : m_path(path), m_index(index), m_point_index(0) {
    if (m_index != 0 && path.get_command(m_index) == Command::Move) m_index--;

    GK_ASSERT(m_index >= 0 && m_index < path.m_commands_size, "Index out of range.");

    if (m_index < path.m_commands_size / 2) {
      for (size_t i = 0; i < m_index; i++) {
        switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index += 1;
          break;
        case Command::Quadratic:
          m_point_index += 2;
          break;
        case Command::Cubic:
          m_point_index += 3;
          break;
        }
      }
    } else {
      m_point_index = path.m_points.size();

      for (size_t i = path.m_commands_size - 1; i >= m_index; i--) {
        switch (path.get_command(i)) {
        case Command::Move:
        case Command::Line:
          m_point_index -= 1;
          break;
        case Command::Quadratic:
          m_point_index -= 2;
          break;
        case Command::Cubic:
          m_point_index -= 3;
          break;
        }
      }
    }
  }

  Path::ReverseIterator& Path::ReverseIterator::operator++() {
    GK_ASSERT(m_index > 0, "Cannot increment the rend iterator.");

    m_index -= 1;

    switch (m_path.get_command(m_index)) {
    case Command::Move:
      if (m_index > 0) operator++();
    case Command::Line:
      m_point_index -= 1;
      break;
    case Command::Quadratic:
      m_point_index -= 2;
      break;
    case Command::Cubic:
      m_point_index -= 3;
      break;
    }

    return *this;
  }

  Path::ReverseIterator Path::ReverseIterator::operator++(int) {
    ReverseIterator tmp = *this;
    ++(*this);

    return tmp;
  }

  Path::ReverseIterator& Path::ReverseIterator::operator--() {
    GK_ASSERT(m_index < m_path.m_commands_size, "Cannot decrement the rbegin iterator.");

    switch (m_path.get_command(m_index)) {
    case Command::Move:
    case Command::Line:
      m_point_index += 1;
      break;
    case Command::Quadratic:
      m_point_index += 2;
      break;
    case Command::Cubic:
      m_point_index += 3;
      break;
    }

    m_index += 1;

    if (m_index < m_path.m_commands_size && m_path.get_command(m_index) == Command::Move) operator++();

    return *this;
  }

  Path::ReverseIterator Path::ReverseIterator::operator--(int) {
    ReverseIterator tmp = *this;
    --(*this);

    return tmp;
  }

  Path::ReverseIterator::value_type Path::ReverseIterator::operator*() const {
    const Command command = m_path.get_command(m_index);

    switch (command) {
    case Command::Cubic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 2 < m_path.m_points.size(), "Not enough points for a cubic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1], m_path.m_points[m_point_index + 2] };
    }
    case Command::Quadratic: {
      GK_ASSERT(m_point_index > 0 && m_point_index + 1 < m_path.m_points.size(), "Not enough points for a quadratic bezier.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index], m_path.m_points[m_point_index + 1] };
    }
    case Command::Line: {
      GK_ASSERT(m_point_index > 0 && m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index - 1], m_path.m_points[m_point_index] };
    }
    default:
    case Command::Move:
      GK_ASSERT(m_point_index < m_path.m_points.size(), "Points vector subscript out of range.");
      return { m_path.m_points[m_point_index] };
    }
  }

  /* -- Path -- */

  Path::Path() : m_points(), m_commands() {}

  Path::Path(const Path& other) :
    m_points(other.m_points), m_commands(other.m_commands), m_commands_size(other.m_commands_size),
    m_closed(other.m_closed), m_in_handle(other.m_in_handle), m_out_handle(other.m_out_handle) {}

  Path::Path(Path&& other) noexcept :
    m_points(std::move(other.m_points)), m_commands(std::move(other.m_commands)), m_commands_size(other.m_commands_size),
    m_closed(other.m_closed), m_in_handle(other.m_in_handle), m_out_handle(other.m_out_handle) {}

  Path::Path(io::DataDecoder& decoder) {
    m_commands = decoder.vector<uint8_t>();

    if (m_commands.empty()) {
      m_commands_size = 0;
      m_closed = false;
      return;
    }

    m_points = decoder.vector<vec2>();
    m_closed = decoder.boolean();

    size_t point_index = 0;
    size_t last_non_move_index = 0;
    size_t last_non_move_point_index = 0;

    for (size_t i = 0; i < m_commands.size() * 4; i++) {
      Command command = get_command(i);

      switch (command) {
      case Command::Move:
      case Command::Line:
        point_index += 1;
        break;
      case Command::Quadratic:
        point_index += 2;
        break;
      case Command::Cubic:
        point_index += 3;
        break;
      }

      if (command != Command::Move) {
        last_non_move_index = i;
        last_non_move_point_index = point_index;
      }
    }

    if (last_non_move_index == 0) {
      m_commands_size = 1;

      m_commands.resize(1);
      m_points.resize(1);

      m_in_handle = decoder.vec2();
      m_out_handle = decoder.vec2();

      return;
    }

    m_commands_size = last_non_move_index + 1;
    m_points.resize(last_non_move_point_index);

    m_in_handle = decoder.vec2();
    m_out_handle = decoder.vec2();
  }

  Path& Path::operator=(const Path& other) {
    m_points = other.m_points;
    m_commands = other.m_commands;
    m_commands_size = other.m_commands_size;
    m_closed = other.m_closed;
    m_in_handle = other.m_in_handle;
    m_out_handle = other.m_out_handle;

    return *this;
  }

  Path& Path::operator=(Path&& other) noexcept {
    m_points = std::move(other.m_points);
    m_commands = std::move(other.m_commands);
    m_commands_size = other.m_commands_size;
    m_closed = other.m_closed;
    m_in_handle = other.m_in_handle;
    m_out_handle = other.m_out_handle;

    return *this;
  }

  vec2 Path::point_at(const size_t point_index) const {
    GK_ASSERT(point_index < m_points.size() || point_index == in_handle_index || point_index == out_handle_index, "Point index out of range.");

    switch (point_index) {
    case in_handle_index:
      return m_in_handle;
    case out_handle_index:
      return m_out_handle;
    default:
      return m_points[point_index];
    }
  }

  void Path::for_each(
    std::function<void(const vec2)> move_callback,
    std::function<void(const vec2)> line_callback,
    std::function<void(const vec2, const vec2)> quadratic_callback,
    std::function<void(const vec2, const vec2, const vec2)> cubic_callback
  ) const {
    for (size_t i = 0, j = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Cubic:
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        if (cubic_callback) {
          cubic_callback(m_points[j], m_points[j + 1], m_points[j + 2]);
        }

        j += 3;
        break;
      case Command::Quadratic:
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        if (quadratic_callback) {
          quadratic_callback(m_points[j], m_points[j + 1]);
        }

        j += 2;
        break;
      case Command::Line:
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        if (line_callback) {
          line_callback(m_points[j]);
        }

        j += 1;
        break;
      case Command::Move:
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        if (move_callback) {
          move_callback(m_points[j]);
        }

        j += 1;
        break;
      }
    }
  }

  void Path::for_each_reversed(
    std::function<void(const vec2)> move_callback,
    std::function<void(const vec2, const vec2)> line_callback,
    std::function<void(const vec2, const vec2, const vec2)> quadratic_callback,
    std::function<void(const vec2, const vec2, const vec2, const vec2)> cubic_callback
  ) const {
    for (int i = static_cast<int>(m_commands_size) - 1, j = static_cast<int>(m_points.size()); i >= 0; i--) {
      switch (get_command(i)) {
      case Command::Cubic:
        GK_ASSERT(j - 4 >= 0, "Not enough points for a cubic bezier.");

        if (cubic_callback) {
          cubic_callback(m_points[j - 4], m_points[j - 3], m_points[j - 2], m_points[j - 1]);
        }

        j -= 3;
        break;
      case Command::Quadratic:
        GK_ASSERT(j - 3 >= 0, "Not enough points for a quadratic bezier.");

        if (quadratic_callback) {
          quadratic_callback(m_points[j - 3], m_points[j - 2], m_points[j - 1]);
        }

        j -= 2;
        break;
      case Command::Line:
        GK_ASSERT(j - 2 >= 0, "Not enough points for a line.");

        if (line_callback) {
          line_callback(m_points[j - 2], m_points[j - 1]);
        }

        j -= 1;
        break;
      case Command::Move:
        GK_ASSERT(j - 1 >= 0, "Points vector subscript out of range.");

        if (move_callback) {
          move_callback(m_points[j - 1]);
        }

        j -= 1;
        break;
      }
    }
  }

  size_t Path::size() const {
    size_t size = 0;

    for (size_t i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move:
        break;
      default:
        size += 1;
      }
    }

    return size;
  }

  size_t Path::points_size(const bool include_handles) const {
    if (include_handles) {
      return m_points.size();
    }

    return m_commands_size;
  }

  std::vector<size_t> Path::vertex_indices() const {
    std::vector<size_t> indices;

    indices.reserve(points_size(false));

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move:
        indices.push_back(point_i);
        point_i += 1;
        break;
      case Command::Line:
        indices.push_back(point_i);
        point_i += 1;
        break;
      case Command::Quadratic:
        indices.push_back(point_i + 1);
        point_i += 2;
        break;
      case Command::Cubic:
        indices.push_back(point_i + 2);
        point_i += 3;
        break;
      }
    }

    if (closed()) {
      indices.pop_back();
    }

    return indices;
  }

  bool Path::is_vertex(const size_t point_index) const {
    if (point_index == 0) return true;

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
      if (point_i > point_index) return false;

      switch (get_command(i)) {
      case Command::Move:
      case Command::Line:
        point_i += 1;
        break;
      case Command::Quadratic:
        point_i += 2;
        break;
      case Command::Cubic:
        point_i += 3;
        break;
      }

      if (point_i - 1 == point_index) {
        return true;
      }
    }

    return false;
  }

  bool Path::is_open_end(const size_t point_index) const {
    return (point_index == 0 || point_index == m_points.size() - 1) && !closed();
  }

  Path::VertexNode Path::node_at(const size_t point_index) const {
    GK_ASSERT(point_index < m_points.size() || point_index == in_handle_index || point_index == out_handle_index, "Point index out of range.");

    VertexNode node = { 0, -1, -1, -1, -1, -1 };

    if (empty()) {
      if (!vacant()) {
        node.vertex = 0;
        node.in = in_handle_index;
        node.out = out_handle_index;
      }

      return node;
    }

    if (!closed()) {
      switch (point_index) {
      case in_handle_index:
        node.out = in_handle_index;
        node.vertex = 0;

        if (m_commands_size > 1 && get_command(1) == Command::Cubic) {
          node.in = 1;
          node.in_command = 1;
        } else if (m_commands_size == 1) {
          node.in = out_handle_index;
        }

        return node;
      case out_handle_index:
        node.out = out_handle_index;
        node.vertex = m_points.size() - 1;

        if (m_commands_size > 1 && get_command(m_commands_size - 1) == Command::Cubic) {
          node.in = m_points.size() - 2;
          node.in_command = m_commands_size - 1;
        } else if (m_commands_size == 1) {
          node.in = in_handle_index;
        }

        return node;
      default:
        break;
      }
    }

    // if (closed() && point_index == m_points.size() - 1) {
    //   point_index = 0;
    // }

    Iterator it = { *this, point_index, IndexType::Point };
    Segment segment = *it;

    int64_t* in = &node.in;
    int64_t* in_command = &node.in_command;
    int64_t* out = &node.out;
    int64_t* out_command = &node.out_command;

    if (point_index != 0 && !(segment.type == Command::Cubic && it.point_index() >= point_index)) {
      it++;

      if (point_index == m_points.size() - 1 || it != end()) {
        in = &node.out;
        in_command = &node.out_command;
        out = &node.in;
        out_command = &node.in_command;
      }

      if (it == end()) {
        node.vertex = it.point_index() - 1;
        *out_command = m_commands_size - 1;

        if (segment.type == Command::Cubic) {
          *out = node.vertex - 1;
        }

        if (closed()) {
          *in_command = 1;

          if (get_command(1) == Command::Cubic) {
            *in = 1;
          }

          node.close_vertex = 0;
        } else {
          *in = out_handle_index;
        }

        return node;
      } else {
        segment = *it;
      }
    }

    node.vertex = it.point_index() - 1;
    *out_command = it.command_index();

    if (segment.type == Command::Cubic) {
      *out = node.vertex + 1;
    }

    if (it.segment_index() > 0) {
      Iterator prev_it = it - 1;
      Segment prev_segment = *prev_it;

      *in_command = prev_it.command_index();

      if (prev_segment.type == Command::Cubic) {
        *in = node.vertex - 1;
      }
    } else if (closed()) {
      *in_command = m_commands_size - 1;

      if (get_command(m_commands_size - 1) == Command::Cubic) {
        *in = m_points.size() - 2;
      }

      node.close_vertex = m_points.size() - 1;
    } else {
      *in = in_handle_index;
    }

    return node;
  }

  void Path::move_to(const vec2 point) {
    GK_ASSERT(empty(), "Cannot add a move to a non-empty path.");

    if (!vacant() && get_command(m_commands_size - 1) == Command::Move) {
      m_points[m_points.size() - 1] = point;
      return;
    }

    m_points.push_back(point);
    m_in_handle = point;
    m_out_handle = point;

    push_command(Command::Move);
  }

  void Path::line_to(const vec2 point, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add a line to a vacant path.");

    if (reverse) {
      m_points.insert(m_points.begin(), point);
      m_in_handle = point;

      insert_command(Command::Line, 0);
    } else {
      m_points.push_back(point);
      m_out_handle = point;

      push_command(Command::Line);
    }
  }

  void Path::quadratic_to(const vec2 control, const vec2 point, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add a quadratic bezier to a vacant path.");

    if (reverse) {
      m_points.insert(m_points.begin(), { point, control });
      m_in_handle = point;

      insert_command(Command::Quadratic, 0);
    } else {
      m_points.insert(m_points.end(), { control, point });
      m_out_handle = point;

      push_command(Command::Quadratic);
    }
  }

  void Path::cubic_to(const vec2 control1, const vec2 control2, const vec2 point, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    if (control1 == (reverse ? m_points.front() : m_points.back()) && control2 == point) {
      return line_to(point, reverse);
    }

    if (reverse) {
      m_points.insert(m_points.begin(), { point, control2, control1 });
      m_in_handle = point;

      insert_command(Command::Cubic, 0);
    } else {
      m_points.insert(m_points.end(), { control1, control2, point });
      m_out_handle = point;

      push_command(Command::Cubic);
    }
  }

  void Path::cubic_to(const vec2 control, const vec2 point, const bool is_control_1, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add a cubic bezier to a vacant path.");

    if (reverse) {
      if (is_control_1) {
        m_points.insert(m_points.begin(), { point, point, control });
      } else {
        m_points.insert(m_points.begin(), { point, control, m_points.front() });
      }

      m_in_handle = point;

      insert_command(Command::Cubic, 0);
    } else {
      if (is_control_1) {
        m_points.insert(m_points.end(), { control, point, point });
      } else {
        m_points.insert(m_points.end(), { m_points.back(), control, point });
      }

      m_out_handle = point;

      push_command(Command::Cubic);
    }

  }

  void Path::arc_to(const vec2 center, const vec2 radius, const float x_axis_rotation, const bool large_arc_flag, const bool sweep_flag, const vec2 point, const bool reverse) {
    GK_ASSERT(!vacant(), "Cannot add an arc to a vacant path.");

    vec2 r = radius;

    const float sin_th = std::sinf(math::degrees_to_radians(x_axis_rotation));
    const float cos_th = std::cosf(math::degrees_to_radians(x_axis_rotation));

    const vec2 d0 = (center - point) / 2.0f;
    const vec2 d1 = {
      cos_th * d0.x + sin_th * d0.y,
      -sin_th * d0.x + cos_th * d0.y
    };

    const vec2 sq_r = r * r;
    const vec2 sq_p = d1 * d1;

    const float check = sq_p.x / sq_r.x + sq_p.y / sq_r.y;
    if (check > 1.0f) r *= std::sqrtf(check);

    mat2 a = {
      cos_th / r.x, sin_th / r.x,
      -sin_th / r.y, cos_th / r.y
    };
    vec2 p1 = {
      math::dot(a[0], point),
      math::dot(a[1], point)
    };

    const vec2 p0 = {
      math::dot(a[0], center),
      math::dot(a[1], center)
    };

    const float d = math::squared_length(p1 - p0);

    float sfactor_sq = 1.0f / d - 0.25f;
    if (sfactor_sq < 0.0f) sfactor_sq = 0.0f;

    float sfactor = std::sqrtf(sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;

    const vec2 c1 = {
      0.5f * (p0.x + p1.x) - sfactor * (p1.y - p0.y),
      0.5f * (p0.y + p1.y) + sfactor * (p1.x - p0.x)
    };

    const float th0 = std::atan2f(p0.y - c1.y, p0.x - c1.x);
    const float th1 = std::atan2f(p1.y - c1.y, p1.x - c1.x);

    float th_arc = th1 - th0;
    if (th_arc < 0.0f && sweep_flag) th_arc += math::two_pi<float>;
    else if (th_arc > 0.0f && !sweep_flag) th_arc -= math::two_pi<float>;

    int n_segs = static_cast<int>(std::ceilf(std::fabsf(th_arc / (0.5f * math::pi<float> +math::geometric_epsilon<float>))));
    for (int i = 0; i < n_segs; i++) {
      const float th2 = th0 + i * th_arc / n_segs;
      const float th3 = th0 + (i + 1) * th_arc / n_segs;

      a = {
        cos_th * r.x, -sin_th * r.x,
        sin_th * r.y, cos_th * r.y
      };

      const float th_half = 0.5f * (th3 - th2);
      const float sin_half_th_half = std::sinf(th_half * 0.5f);
      const float t = (8.0f / 3.0f) * sin_half_th_half * sin_half_th_half / std::sin(th_half);

      const float sin_th2 = std::sinf(th2);
      const float cos_th2 = std::cosf(th2);
      const float sin_th3 = std::sinf(th3);
      const float cos_th3 = std::cosf(th3);

      p1 = {
        c1.x + cos_th2 - t * sin_th2,
        c1.y + sin_th2 + t * cos_th2
      };

      const vec2 p3 = {
        c1.x + cos_th3,
        c1.y + sin_th3
      };
      const vec2 p2 = {
        p3.x + t * sin_th3,
        p3.y - t * cos_th3
      };

      const vec2 bez1 = {
        math::dot(a[0], p1),
        math::dot(a[1], p1)
      };
      const vec2 bez2 = {
        math::dot(a[0], p2),
        math::dot(a[1], p2)
      };
      const vec2 bez3 = {
        math::dot(a[0], p3),
        math::dot(a[1], p3)
      };

      cubic_to(bez1, bez2, bez3, reverse);
    }
  }

  void Path::ellipse(const vec2 center, const vec2 radius) {
    const vec2 top_left = center - radius;
    const vec2 bottom_right = center + radius;
    const vec2 cp = radius * math::circle_ratio<float>;

    move_to({ center.x, top_left.y });
    cubic_to({ center.x + cp.x, top_left.y }, { bottom_right.x, center.y - cp.y }, { bottom_right.x, center.y });
    cubic_to({ bottom_right.x, center.y + cp.y }, { center.x + cp.x, bottom_right.y }, { center.x, bottom_right.y });
    cubic_to({ center.x - cp.x, bottom_right.y }, { top_left.x, center.y + cp.y }, { top_left.x, center.y });
    cubic_to({ top_left.x, center.y - cp.y }, { center.x - cp.x, top_left.y }, { center.x, top_left.y });
    close();
  }

  void Path::circle(const vec2 center, const float radius) {
    ellipse(center, { radius, radius });
  }

  void Path::rect(const vec2 point, const vec2 size, const bool centered) {
    vec2 p = point;

    if (centered) {
      p -= size * 0.5f;
    }

    move_to(p);
    line_to(p + vec2{ size.x, 0.0f });
    line_to(p + size);
    line_to(p + vec2{ 0.0f, size.y });
    close();
  }

  void Path::round_rect(const vec2 point, const vec2 size, const float radius, const bool centered) {
    float r = radius;
    vec2 p = point;

    if (centered) {
      p -= size * 0.5f;
    }

    if (r > size.x * 0.5f) r = size.x * 0.5f;
    if (r > size.y * 0.5f) r = size.y * 0.5f;

    move_to({ p.x + r, p.y });
    line_to({ p.x + size.x - r, p.y });
    cubic_to({ p.x + size.x - r * math::circle_ratio<float>, p.y }, { p.x + size.x, p.y + r * math::circle_ratio<float> }, { p.x + size.x, p.y + r });
    line_to({ p.x + size.x, p.y + size.y - r });
    cubic_to({ p.x + size.x, p.y + size.y - r * math::circle_ratio<float> }, { p.x + size.x - r * math::circle_ratio<float>, p.y + size.y }, { p.x + size.x - r, p.y + size.y });
    line_to({ p.x + r, p.y + size.y });
    cubic_to({ p.x + r * math::circle_ratio<float>, p.y + size.y }, { p.x, p.y + size.y - r * math::circle_ratio<float> }, { p.x, p.y + size.y - r });
    line_to({ p.x, p.y + r });
    cubic_to({ p.x, p.y + r * math::circle_ratio<float> }, { p.x + r * math::circle_ratio<float>, p.y }, { p.x + r, p.y });
    close();
  }

  void Path::close() {
    if (empty() || m_commands.empty() || (size() == 1 && get_command(1) == Command::Line)) return;

    const vec2 p = m_points.front();

    if (math::is_almost_equal(m_points.back(), p, math::geometric_epsilon<float>)) {
      m_points[m_points.size() - 1] = p;
    } else {
      const bool has_in = has_in_handle();
      const bool has_out = has_out_handle();

      if (!has_in && !has_out) {
        line_to(p);
      } else {
        cubic_to(m_out_handle, m_in_handle, p);
      }
    }

    m_closed = true;
  }

  void Path::translate(const size_t point_index, const vec2 delta) {
    GK_ASSERT(point_index < m_points.size() || point_index == in_handle_index || point_index == out_handle_index, "Point index out of range.");

    switch (point_index) {
    case in_handle_index:
      m_in_handle += delta;
      break;
    case out_handle_index:
      m_out_handle += delta;
      break;
    default:
      m_points[point_index] += delta;
      break;
    }
  }

  size_t Path::to_line(const size_t command_index, const size_t reference_point) {
    GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

    const Command command = get_command(command_index);
    if (command == Command::Line || command == Command::Move) return reference_point;

    const Iterator it = { *this, command_index, IndexType::Command };
    const Segment segment = *it;
    const size_t point_i = it.point_index();

    if (segment.type == Command::Cubic) {
      m_points.erase(m_points.begin() + point_i, m_points.begin() + point_i + 2);

      replace_command(command_index, Command::Line);

      return reference_point > point_i ? reference_point - 2 : reference_point;
    }

    m_points.erase(m_points.begin() + point_i, m_points.begin() + point_i + 1);

    replace_command(command_index, Command::Line);

    return reference_point > point_i ? reference_point - 1 : reference_point;
  }

  size_t Path::to_cubic(const size_t command_index, size_t reference_point) {
    GK_ASSERT(command_index < m_commands_size, "Command index out of range.");

    const Command command = get_command(command_index);
    if (command == Command::Cubic || command == Command::Move) return reference_point;

    const Iterator it = { *this, command_index, IndexType::Command };
    const Segment segment = *it;
    const size_t point_i = it.point_index();

    if (segment.type == Command::Line) {
      m_points.insert(m_points.begin() + point_i, { m_points[point_i - 1], m_points[point_i] });

      replace_command(command_index, Command::Cubic);

      return reference_point >= point_i ? reference_point + 2 : reference_point;
    }

    const vec2 p0 = m_points[point_i - 1];
    const vec2 p1 = m_points[point_i];
    const vec2 p2 = m_points[point_i + 1];

    const vec2 bez1 = p0 + 2.0f / 3.0f * (p1 - p0);
    const vec2 bez2 = p2 + 2.0f / 3.0f * (p1 - p2);

    m_points[point_i] = bez1;
    m_points.insert(m_points.begin() + point_i + 1, bez2);

    replace_command(command_index, Command::Cubic);

    return reference_point >= point_i + 1 ? reference_point + 1 : reference_point;
  }

  void Path::remove(const size_t point_index, const bool keep_shape) {
    GK_ASSERT(point_index < m_points.size(), "Point index out of range.");

    size_t to_remove = point_index == m_points.size() - 1 ? 0 : point_index;

    if (empty() || (point_index == 0 && !closed())) {
      return;
    }

    const Iterator it = { *this, to_remove, IndexType::Point };
    const Iterator next_it = it + 1;
    const Segment segment = to_remove == 0 ? back() : *it;
    const Segment next_segment = to_remove == 0 ? front() : *next_it;

    if (size() == 2 && closed()) {
      const vec2 p = segment.p0;
      const vec2 out = segment.type == Command::Cubic ? segment.p1 : p;
      const vec2 in = next_segment.type == Command::Cubic ? next_segment.p2 : p;

      m_points.clear();
      m_commands.clear();
      m_commands_size = 0;

      move_to(p);

      m_in_handle = in;
      m_out_handle = out;

      return;
    }

#if 0
    math::Algorithms::CubicBezier cubic;

    if (keep_shape) {
      std::vector<vec2> points(GK_FIT_RESOLUTION * 2 + 2);

      for (size_t i = 0; i <= GK_FIT_RESOLUTION; i++) {
        float t = static_cast<float>(i) / static_cast<float>(GK_FIT_RESOLUTION);

        points[i] = segment.sample(t);
        points[GK_FIT_RESOLUTION + i + 1] = next_segment.sample(t);
      }

      cubic = math::Algorithms::fit_points_to_cubic(points, GK_PATH_TOLERANCE);
    } else {
      const vec2 p1 = segment.type == Command::Line ? segment.p0 : segment.p1;

      if (next_segment.type == Command::Line) {
        cubic = { segment.p0, p1, next_segment.p1, next_segment.p1 };
      } else if (next_segment.type == Command::Quadratic) {
        cubic = { segment.p0, p1, next_segment.p1, next_segment.p2 };
      } else {
        cubic = { segment.p0, p1, next_segment.p2, next_segment.p3 };
      }
    }

    size_t new_command_index = 0;

    if (to_remove == 0) {
      switch (segment.type) {
      case Command::Line:
        m_points.pop_back();
        break;
      case Command::Quadratic:
        m_points.pop_back();
        m_points.pop_back();
        break;
      case Command::Cubic:
        m_points.pop_back();
        m_points.pop_back();
        m_points.pop_back();
        break;
      case Command::Move:
      default:
        break;
      }

      switch (next_segment.type) {
      case Command::Line:
        m_points.insert(m_points.begin(), { cubic.p0, cubic.p1 });
        m_points[2] = cubic.p2;
        break;
      case Command::Quadratic:
        m_points.insert(m_points.begin(), cubic.p0);
        m_points[1] = cubic.p1;
        m_points[2] = cubic.p2;
        break;
      case Command::Cubic:
        m_points[0] = cubic.p0;
        m_points[1] = cubic.p1;
        m_points[2] = cubic.p2;
        break;
      case Command::Move:
      default:
        break;
      }

      new_command_index = 1;

      remove_command(m_commands_size - 1);
      replace_command(new_command_index, Command::Cubic);
    } else {
      switch (segment.type) {
      case Command::Line:
        m_points[to_remove] = cubic.p3;
        m_points.insert(m_points.begin() + to_remove, { cubic.p1, cubic.p2 });
        break;
      case Command::Quadratic:
        m_points[to_remove - 1] = cubic.p1;
        m_points[to_remove] = cubic.p3;
        m_points.insert(m_points.begin() + to_remove, cubic.p2);
        break;
      case Command::Cubic:
        m_points[to_remove - 2] = cubic.p1;
        m_points[to_remove - 1] = cubic.p2;
        m_points[to_remove] = cubic.p3;
        break;
      case Command::Move:
      default:
        break;
      }

      switch (next_segment.type) {
      case Command::Line:
        m_points.erase(m_points.begin() + to_remove + 1);
        break;
      case Command::Quadratic:
        m_points.erase(m_points.begin() + to_remove + 1, m_points.begin() + to_remove + 3);
        break;
      case Command::Cubic:
        m_points.erase(m_points.begin() + to_remove + 1, m_points.begin() + to_remove + 4);
        break;
      case Command::Move:
      default:
        break;
      }

      new_command_index = it.command_index();

      replace_command(new_command_index, Command::Cubic);
      remove_command(next_it.command_index());
    }

    if (cubic.p0 == cubic.p1 && cubic.p2 == cubic.p3) {
      to_line(new_command_index);
    }
#endif
  }

  size_t Path::split(const size_t segment_index, const float t) {
    GK_ASSERT(segment_index < m_commands_size - 1, "Segment index out of range.");

    if (empty()) return 0;

    const Iterator it = { *this, segment_index, IndexType::Segment };
    const Segment segment = *it;
    const size_t point_i = it.point_index();

    switch (segment.type) {
    case Command::Line: {
      const vec2 p = math::lerp(segment.p0, segment.p1, t);

      m_points.insert(m_points.begin() + point_i, p);
      insert_command(Command::Line, segment_index + 1);

      return point_i;
    }
    case Command::Quadratic:
      return point_i + 1;
    case Command::Cubic: {

#if 0
      const auto [p, in_p1, in_p2, out_p1, out_p2] = math::split_bezier(
        m_points[point_i - 1], m_points[point_i], m_points[point_i + 1], m_points[point_i + 2], t
      );

      m_points[point_i] = in_p1;
      m_points[point_i + 1] = out_p2;

      m_points.insert(m_points.begin() + point_i + 1, { in_p2, p, out_p1 });
      insert_command(Command::Cubic, segment_index + 1);
#endif

      return point_i + 2;
    }
    case Command::Move:
    default:
      break;
    };

    return 0;
  }

  math::rect Path::bounding_rect() const {
    if (empty()) {
      if (vacant()) return {};
      return { m_points[0], m_points[0] };
    }

    math::rect rect{};

    for (size_t i = 0, j = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j > 0, "Cubic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        const vec2 p0 = m_points[j - 1];
        const vec2 p1 = m_points[j];
        const vec2 p2 = m_points[j + 1];
        const vec2 p3 = m_points[j + 2];

        math::rect r = geom::bounding_rect(cubic_bezier{ p0, p1, p2, p3 });

        math::min(rect.min, r.min, rect.min);
        math::max(rect.max, r.max, rect.max);

        j += 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j > 0, "Quadratic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        const vec2 p0 = m_points[j - 1];
        const vec2 p1 = m_points[j];
        const vec2 p2 = m_points[j + 1];

        math::rect r = geom::bounding_rect(quadratic_bezier{ p0, p1, p2 });

        math::min(rect.min, r.min, rect.min);
        math::max(rect.max, r.max, rect.max);

        j += 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j > 0, "Line command cannot be the first command of a path.");
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        math::min(rect.min, m_points[j], rect.min);
        math::max(rect.max, m_points[j], rect.max);

        j += 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        math::min(rect.min, m_points[j], rect.min);
        math::max(rect.max, m_points[j], rect.max);

        j += 1;

        break;
      }
      }
    }

    return rect;
  }

  math::rect Path::bounding_rect(const mat2x3& transform) const {
    if (empty()) {
      if (vacant()) return {};

      const vec2 p = transform * m_points[0];
      return { p, p };
    }

    math::rect rect{};

    for (size_t i = 0, j = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Cubic: {
        GK_ASSERT(j > 0, "Cubic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 2 < m_points.size(), "Not enough points for a cubic bezier.");

        const vec2 p0 = transform * m_points[j - 1];
        const vec2 p1 = transform * m_points[j];
        const vec2 p2 = transform * m_points[j + 1];
        const vec2 p3 = transform * m_points[j + 2];

        math::rect r = geom::bounding_rect(cubic_bezier{ p0, p1, p2, p3 });

        math::min(rect.min, r.min, rect.min);
        math::max(rect.max, r.max, rect.max);

        j += 3;

        break;
      }
      case Command::Quadratic: {
        GK_ASSERT(j > 0, "Quadratic bezier command cannot be the first command of a path.");
        GK_ASSERT(j + 1 < m_points.size(), "Not enough points for a quadratic bezier.");

        const vec2 p0 = transform * m_points[j - 1];
        const vec2 p1 = transform * m_points[j];
        const vec2 p2 = transform * m_points[j + 1];

        math::rect r = geom::bounding_rect(quadratic_bezier{ p0, p1, p2 });

        math::min(rect.min, r.min, rect.min);
        math::max(rect.max, r.max, rect.max);

        j += 2;

        break;
      }
      case Command::Line: {
        GK_ASSERT(j > 0, "Line command cannot be the first command of a path.");
        GK_ASSERT(j < m_points.size(), "Not enough points for a line.");

        const vec2 p1 = transform * m_points[j];

        math::min(rect.min, p1, rect.min);
        math::max(rect.max, p1, rect.max);

        j += 1;

        break;
      }
      case Command::Move: {
        GK_ASSERT(j < m_points.size(), "Points vector subscript out of range.");

        const vec2 p0 = transform * m_points[j];

        math::min(rect.min, p0, rect.min);
        math::max(rect.max, p0, rect.max);

        j += 1;

        break;
      }
      }
    }

    return rect;
  }

  math::rect Path::approx_bounding_rect() const {
    if (empty()) {
      if (vacant()) return {};
      return { m_points[0], m_points[0] };
    }

    math::rect rect{};

    for (vec2 p : m_points) {
      math::min(rect.min, p, rect.min);
      math::max(rect.max, p, rect.max);
    }

    math::min(rect.min, m_in_handle, rect.min);
    math::max(rect.max, m_in_handle, rect.max);
    math::min(rect.min, m_out_handle, rect.min);
    math::max(rect.max, m_out_handle, rect.max);

    return rect;
  }

  bool Path::is_point_inside_path(const vec2 point, const renderer::Fill* fill, const renderer::Stroke* stroke, const mat2x3& transform, const float threshold, const double zoom, const bool deep_search) const {
    GK_TOTAL("Path::is_point_inside_path");

    if (empty()) {
      if (vacant()) {
        return false;
      }

      return (geom::is_point_in_circle(point, transform * m_points[0], threshold) || (deep_search && (
        geom::is_point_in_circle(point, transform * m_in_handle, threshold) ||
        geom::is_point_in_circle(point, transform * m_out_handle, threshold)))
      );
    }

#if 0
    const math::rect bounds = approx_bounding_rect();
    const bool consider_miters = stroke ? (stroke->join == renderer::LineJoin::Miter) && (stroke->width > threshold) : false;

    if (!is_point_in_rect(math::inverse(transform) * point, bounds, stroke ? 0.5f * stroke->width * (consider_miters ? stroke->miter_limit : 1.0f) + threshold : threshold)) return false;

    const math::rect threshold_box = { point - threshold - math::geometric_epsilon<float> / zoom, point + threshold + math::geometric_epsilon<float> / zoom };
    const f24x8x2 p = { math::float_to_f24x8(point.x), math::float_to_f24x8(point.y) };

    const renderer::geometry::QuadraticPath quadratics = to_quadratics();

    graphick::renderer::geometry::PathBuilder builder{ quadratics, transform, &bounds };

    // PathBuilder builder{ threshold_box, dmat2x3(transform), GK_PATH_TOLERANCE / zoom };

    // TODO: fix all of this mess, maybe with specific methods in pathbuilder

    if (fill) {
      std::vector<vec4> lines;

      // TODO: clip here with specific fill clipping method
      builder.flatten(bounds, GK_PATH_TOLERANCE / zoom, lines);

      lines.emplace_back(lines.back().z, lines.back().w, lines.front().x, lines.front().y);

      // TEMP
      Contour contour;

      contour.move_to(dvec2{ lines[0].x, lines[0].y });

      for (vec4 line : lines) {
        contour.line_to(dvec2{ line.z, line.w });
      }

      const int winding = contour.winding_of(p);

      if (
        (fill->rule == FillRule::NonZero && winding != 0) ||
        (fill->rule == FillRule::EvenOdd && winding % 2 != 0)
        ) {
        return true;
      }
    }

    Stroke s = stroke ? *stroke : Stroke{ vec4{}, LineCap::Round, LineJoin::Round, 0.0f, 0.0f, 0.0f };
    s.width += threshold;

    if (!consider_miters) {
      s.miter_limit = 0.0f;
    }

    renderer::geometry::PathBuilder::StrokeOutline contours = builder.stroke(s, GK_PATH_TOLERANCE / zoom);

    // Drawable drawable = builder.stroke(*this, s);

    int winding = 0;

    if (!contours.inner.empty()) {
      std::vector<vec4> lines;

      graphick::renderer::geometry::PathBuilder stroke_builder{ contours.inner, { } };

      stroke_builder.flatten(threshold_box, GK_PATH_TOLERANCE / zoom, lines);

      if (!lines.empty()) {
        // TEMP
        Contour contour;

        contour.move_to(dvec2{ lines[0].x, lines[0].y });

        for (vec4 line : lines) {
          contour.line_to(dvec2{ line.z, line.w });
        }

        winding += contour.winding_of(p);
      }
    }

    if (!contours.outer.empty()) {
      std::vector<vec4> lines;

      graphick::renderer::geometry::PathBuilder stroke_builder{ contours.outer, { } };

      stroke_builder.flatten(threshold_box, GK_PATH_TOLERANCE / zoom, lines);

      if (!lines.empty()) {
        // TEMP
        Contour contour;

        contour.move_to(dvec2{ lines[0].x, lines[0].y });

        for (vec4 line : lines) {
          contour.line_to(dvec2{ line.z, line.w });
        }

        winding += contour.winding_of(p);
      }
    }

    if (winding != 0) {
      return true;
    }

    // for (Contour& contour : drawable.contours) {
    //   const int winding = contour.winding_of(p);

    //   if (winding != 0) {
    //     return true;
    //   }
    // }

    for (size_t point_index = 0; point_index < m_points.size(); point_index++) {
      if (math::is_point_in_circle(point, transform * m_points[point_index], threshold)) {
        if (deep_search || point_index == 0) return true;

        for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
          switch (get_command(i)) {
          case Command::Move:
          case Command::Line:
            if (point_i == point_index) {
              return true;
            }

            point_i += 1;

            break;
          case Command::Quadratic:
            if (point_i + 1 == point_index) {
              return true;
            }

            point_i += 2;

            break;
          case Command::Cubic:
            if (point_i + 2 == point_index) {
              return true;
            }

            point_i += 3;

            break;
          }
        }

        return false;
      }
    }

    return (deep_search && (
      math::is_point_in_circle(point, transform * m_in_handle, threshold) ||
      math::is_point_in_circle(point, transform * m_out_handle, threshold))
    );
#endif
  }

  bool Path::is_point_inside_segment(const size_t segment_index, const vec2 point, const renderer::Stroke* stroke, const mat2x3& transform, const float threshold, const double zoom) const {
#if 0
    const Segment segment = at(segment_index);
    const math::rect bounds = segment.approx_bounding_rect();

    if (!math::is_point_in_rect(math::inverse(transform) * point, bounds, stroke ? 0.5f * stroke->width + threshold : threshold)) return false;

    const math::rect threshold_box = { point - threshold - math::geometric_epsilon<float> / zoom, point + threshold + math::geometric_epsilon<float> / zoom };
    const f24x8x2 p = { math::float_to_f24x8(point.x), math::float_to_f24x8(point.y) };

    // PathBuilder builder{ threshold_box, dmat2x3(transform), GK_PATH_TOLERANCE / zoom };

    Stroke s = stroke ? *stroke : Stroke{ vec4{}, LineCap::Butt, LineJoin::Bevel, 0.0f, 0.0f, 0.0f };
    s.width += threshold;

    Path segment_path;

    segment_path.move_to(segment.p0);

    switch (segment.type) {
    case Command::Cubic:
      segment_path.cubic_to(segment.p1, segment.p2, segment.p3);
      break;
    case Command::Quadratic:
      segment_path.quadratic_to(segment.p1, segment.p2);
      break;
    case Command::Line:
      segment_path.line_to(segment.p1);
      break;
    default:
      break;
    }

    return segment_path.is_point_inside_path(point, nullptr, &s, transform, threshold, zoom, false);

    // graphick::renderer::geometry::PathBuilder builder{ segment_path.to_quadratics(), transform, &bounds };

    // Drawable drawable = builder.stroke(segment_path, s);

    // for (Contour& contour : drawable.contours) {
    //   const int winding = contour.winding_of(p);

    //   if (winding != 0) {
    //     return true;
    //   }
    // }

    // return false;
#endif
    return false;
  }

  bool Path::is_point_inside_point(const size_t point_index, const vec2 point, const mat2x3& transform, const float threshold) const {
    const vec2 p = transform * point_at(point_index);

    if (geom::is_point_in_circle(point, p, threshold)) {
      if (point_index == 0) return true;

      for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
        switch (get_command(i)) {
        case Command::Move:
        case Command::Line:
          if (point_i == point_index) {
            return true;
          }

          point_i += 1;

          break;
        case Command::Quadratic:
          if (point_i == point_index) {
            if (m_points[point_i] == m_points[point_i - 1] || m_points[point_i] == m_points[point_i + 1]) {
              return false;
            }

            return true;
          } else if (point_i + 1 == point_index) {
            return true;
          }

          point_i += 2;

          break;
        case Command::Cubic:
          if (point_i == point_index) {
            if (m_points[point_i] == m_points[point_i - 1] || m_points[point_i] == m_points[point_i + 2]) {
              return false;
            }

            return true;
          } else if (point_i + 1 == point_index) {
            if (m_points[point_i + 1] == m_points[point_i - 1] || m_points[point_i + 1] == m_points[point_i + 2]) {
              return false;
            }

            return true;
          } else if (point_i + 2 == point_index) {
            return true;
          }

          point_i += 3;

          break;
        }
      }

      if (point_index == in_handle_index && !has_in_handle()) return false;
      if (point_index == out_handle_index && !has_out_handle()) return false;

      return true;
    }

    return false;
  }

  bool Path::intersects(const math::rect& rect, std::unordered_set<size_t>* indices) const {
    if (m_commands_size == 0) {
      return false;
    } else if (m_commands_size == 1) {
      if (geom::is_point_in_rect(m_points[0], rect)) {
        if (indices) indices->insert(0);
        return true;
      }

      return false;
    }

    if (!geom::does_rect_intersect_rect(rect, approx_bounding_rect())) {
      return false;
    }

    bool found = false;

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move:
        if (geom::is_point_in_rect(m_points[point_i], rect)) {
          if (indices) indices->insert(point_i);
          found = true;
        }

        point_i += 1;
        break;
      case Command::Line:
        if (geom::is_point_in_rect(m_points[point_i], rect)) {
          if (indices) indices->insert(point_i);
          found = true;
        } else if (!found && geom::does_line_intersect_rect({ m_points[point_i - 1], m_points[point_i] }, rect)) {
          found = true;
        }

        point_i += 1;
        break;
      case Command::Quadratic:
        if (geom::is_point_in_rect(m_points[point_i + 1], rect)) {
          if (indices) indices->insert(point_i + 1);
          found = true;
        } else if (!found && geom::does_quadratic_intersect_rect({ m_points[point_i - 1], m_points[point_i], m_points[point_i + 1] }, rect)) {
          found = true;
        }

        point_i += 2;
        break;
      case Command::Cubic:
        if (geom::is_point_in_rect(m_points[point_i + 2], rect)) {
          if (indices) indices->insert(point_i + 2);
          found = true;
        } else if (!found && geom::does_cubic_intersect_rect({ m_points[point_i - 1], m_points[point_i], m_points[point_i + 1], m_points[point_i + 2] }, rect)) {
          found = true;
        }

        point_i += 3;
        break;
      }
    }

    if (indices && closed()) {
      indices->erase(m_points.size() - 1);
    }

    return found;
  }

  bool Path::intersects(const math::rect& rect, const mat2x3& transform, std::unordered_set<size_t>* indices) const {
    if (m_commands_size == 0) {
      return false;
    } else if (m_commands_size == 1) {
      if (geom::is_point_in_rect(transform * m_points[0], rect)) {
        if (indices) indices->insert(0);
        return true;
      }

      return false;
    }

    vec2 last = { 0.0f, 0.0f };

    if (!geom::does_rect_intersect_rect(rect, transform * approx_bounding_rect())) {
      return false;
    }

    bool found = false;

    for (size_t i = 0, point_i = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move: {
        const vec2 p0 = transform * m_points[point_i];

        if (geom::is_point_in_rect(p0, rect)) {
          if (indices) indices->insert(point_i);
          found = true;
        }

        point_i += 1;
        last = p0;

        break;
      }
      case Command::Line: {
        const vec2 p1 = transform * m_points[point_i];

        if (geom::is_point_in_rect(p1, rect)) {
          if (indices) indices->insert(point_i);
          found = true;
        } else if (!found && geom::does_line_intersect_rect({ last, p1 }, rect)) {
          found = true;
        }

        point_i += 1;
        break;
      }
      case Command::Quadratic: {
        const vec2 p2 = transform * m_points[point_i + 1];

        if (geom::is_point_in_rect(p2, rect)) {
          if (indices) indices->insert(point_i + 1);
          found = true;
        } else if (!found && geom::does_quadratic_intersect_rect({ last, transform * m_points[point_i], p2 }, rect)) {
          found = true;
        }

        point_i += 2;
        last = p2;

        break;
      }
      case Command::Cubic: {
        const vec2 p1 = transform * m_points[point_i];
        const vec2 p2 = transform * m_points[point_i + 1];
        const vec2 p3 = transform * m_points[point_i + 2];

        if (geom::is_point_in_rect(p3, rect)) {
          if (indices) indices->insert(point_i + 2);
          found = true;
        } else if (!found && geom::does_cubic_intersect_rect({ last, p1, p2, p3 }, rect)) {
          found = true;
        }

        point_i += 3;
        last = p3;

        break;
      }
      }
    }

    if (indices && closed()) {
      indices->erase(m_points.size() - 1);
    }

    return found;
  }

  enum class CuspType {
    None = 0,
    Loop,
    DoubleInflection
  };

  struct Nearest {
    float t;
    float distance_sq;
  };

  static Path::Segment deriv(const Path::Segment& c) {
    return {
      3.0f * (c.p1 - c.p0),
      3.0f * (c.p2 - c.p1),
      3.0f * (c.p3 - c.p2)
    };
  }

  // static Nearest nearest(const Path::Segment& c, const float accuracy) {
  //   let mut best_r = None;
  //   let mut best_t = 0.0;

  //   for (t0, t1, q) in self.to_quads(accuracy) {
  //     let nearest = q.nearest(p, accuracy);
  //     if best_r
  //       .map(| best_r | nearest.distance_sq < best_r)
  //       .unwrap_or(true)
  //     {
  //       best_t = t0 + nearest.t * (t1 - t0);
  //       best_r = Some(nearest.distance_sq);
  //     }
  //   }
  //   Nearest{
  //       t: best_t,
  //       distance_sq : best_r.unwrap(),
  //   }
  // }

  // static CuspType detect_cusp(const Path::Segment& c, const float dimension) {
  //   const vec2 d01 = c.p1 - c.p0;
  //   const vec2 d02 = c.p2 - c.p0;
  //   const vec2 d03 = c.p3 - c.p0;
  //   const vec2 d12 = c.p2 - c.p1;
  //   const vec2 d23 = c.p3 - c.p2;

  //   float det_012 = math::cross(d01, d02);
  //   float det_123 = math::cross(d12, d23);
  //   float det_013 = math::cross(d01, d03);
  //   float det_023 = math::cross(d02, d03);

  //   if (det_012 * det_123 > 0.0f && det_012 * det_013 < 0.0f && det_012 * det_023 < 0.0f) {
  //     Path::Segment q = deriv(c);

  //     // Accuracy isn't used for quadratic nearest
  //     let nearest = q.nearest(Point::ORIGIN, 1e-9);
  //     // detect whether curvature at minimum derivative exceeds 1/dimension,
  //     // without division.
  //     let d = q.eval(nearest.t);
  //     let d2 = q.deriv().eval(nearest.t);
  //     let cross = d.to_vec2().cross(d2.to_vec2());
  //     if nearest.distance_sq.powi(3) <= (cross * dimension).powi(2) {
  //       let a = 3. * det_012 + det_023 - 2. * det_013;
  //       let b = -3. * det_012 + det_013;
  //       let c = det_012;
  //       let d = b * b - 4. * a * c;
  //       if d > 0.0f {
  //         return Some(CuspType::DoubleInflection);
  //       } else {
  //         return Some(CuspType::Loop);
  //       }
  //     }
  //   }
  //   None
  // }

  // TODO: move to maths directory
  // static Path::Segment regularize(Path::Segment c, const float dimension) {
  //   const float dim2 = dimension * dimension;

  //   if (math::squared_distance(c.p0, c.p1) < dim2) {
  //     const float d02 = math::squared_distance(c.p0, c.p2);

  //     if (d02 >= dim2) {
  //       c.p1 = math::lerp(c.p0, c.p2, std::sqrtf(dim2 / d02));
  //     } else {
  //       c.p1 = math::lerp(c.p0, c.p3, 1.0f / 3.0f);
  //       c.p2 = math::lerp(c.p3, c.p0, 1.0f / 3.0f);

  //       return c;
  //     }
  //   }

  //   if (math::squared_distance(c.p3, c.p2) < dim2) {
  //     const float d13 = math::squared_distance(c.p1, c.p2);

  //     if (d13 >= dim2) {
  //       c.p2 = math::lerp(c.p3, c.p1, std::sqrtf(dim2 / d13));
  //     } else {
  //       c.p1 = math::lerp(c.p0, c.p3, 1.0f / 3.0f);
  //       c.p2 = math::lerp(c.p3, c.p0, 1.0f / 3.0f);

  //       return c;
  //     }
  //   }

  //   if

  //     if let Some(cusp_type) = self.detect_cusp(dimension) {
  //       let d01 = c.p1 - c.p0;
  //       let d01h = d01.hypot();
  //       let d23 = c.p3 - c.p2;
  //       let d23h = d23.hypot();
  //       match cusp_type{
  //           CuspType::Loop = > {
  //               c.p1 += (dimension / d01h) * d01;
  //               c.p2 -= (dimension / d23h) * d23;
  //           }
  //           CuspType::DoubleInflection = > {
  //               // Avoid making control distance smaller than dimension
  //               if d01h > 2.0 * dimension {
  //                   c.p1 -= (dimension / d01h) * d01;
  //               }
  //               if d23h > 2.0 * dimension {
  //                   c.p2 += (dimension / d23h) * d23;
  //               }
  //           }
  //       }
  //     }
  //   c
  // }

  /// Computes the point where two lines, if extended to infinity, would cross.
  // static std::optional<vec2> crossing_point(rect& l, rect& other) {
  //   const vec2 ab = l.max - l.min;
  //   const vec2 cd = other.max - other.min;
  //   const float pcd = math::cross(ab, cd);

  //   if (pcd == 0.0f) {
  //     return std::nullopt;
  //   }

  //   const float h = math::cross(ab, (l.min - other.min) / pcd);
  //   return other.min + cd * h;
  // }

  // static void try_approx_quadratic(const Path::Segment& c, const float accuracy, renderer::geometry::QuadraticPath& sink) {
  //   std::optional<vec2> q1 = crossing_point(rect{ c.p0, c.p1 }, rect{ c.p2, c.p3 });

  //   if (q1.has_value()) {
  //     // TODO: implement
  //     // let c1 = self.p0.lerp(q1, 2.0 / 3.0);
  //     // let c2 = self.p3.lerp(q1, 2.0 / 3.0);
  //     // if !CubicBez::new(
  //     //     Point::ZERO,
  //     //     c1 - self.p1.to_vec2(),
  //     //     c2 - self.p2.to_vec2(),
  //     //     Point::ZERO,
  //     // )
  //     //   .fit_inside(accuracy)
  //     // {
  //     //   return None;
  //     // }

  //     sink.quadratic_to(q1.value(), c.p3);
  //   }
  // }

  // static void to_quads(const Path::Segment& c, const float accuracy, renderer::geometry::QuadraticPath& sink) {
  //   // The maximum error, as a vector from the cubic to the best approximating
  //   // quadratic, is proportional to the third derivative, which is constant
  //   // across the segment. Thus, the error scales down as the third power of
  //   // the number of subdivisions. Our strategy then is to subdivide `t` evenly.
  //   //
  //   // This is an overestimate of the error because only the component
  //   // perpendicular to the first derivative is important. But the simplicity is
  //   // appealing.

  //   // This magic number is the square of 36 / sqrt(3).
  //   // See: http://caffeineowl.com/graphics/2d/vectorial/cubic2quad01.html
  //   const float max_hypot2 = 432.0f * accuracy * accuracy;

  //   const vec2 p1x2 = 3.0f * c.p1 - c.p0;
  //   const vec2 p2x2 = 3.0f * c.p2 - c.p3;

  //   const float err = math::squared_length(p2x2 - p1x2);
  //   const int n = std::max(1, static_cast<int>(std::ceilf(std::powf(err / max_hypot2, 1.0f / 6.0f))));

  //   if (n == 1) {
  //     try_approx_quadratic(c, accuracy, sink);
  //     return;
  //   }


  // }

  QuadraticPath Path::to_quadratics(const float tolerance) const {
    GK_TOTAL("Path::to_quadratics");

    QuadraticPath path;

    if (empty()) {
      return path;
    }

    for (size_t i = 0, j = 0; i < m_commands_size; i++) {
      switch (get_command(i)) {
      case Command::Move:
        path.move_to(m_points[j]);
        j += 1;
        break;
      case Command::Line:
        path.line_to(m_points[j]);
        j += 1;
        break;
      case Command::Quadratic:
        path.quadratic_to(m_points[j], m_points[j + 1]);
        j += 2;
        break;
      case Command::Cubic: {
        const geom::cubic_bezier cubic{ m_points[j - 1], m_points[j], m_points[j + 1], m_points[j + 2] };

        std::vector<quadratic_bezier> quads = geom::cubic_to_quadratics(cubic, 2e-2f);

        for (const quadratic_bezier& quad : quads) {
          path.quadratic_to(quad.p1, quad.p2);
        }
#if 0
        math::CubicBezier cubic{ m_points[j - 1], m_points[j], m_points[j + 1], m_points[j + 2] };

        std::vector<vec2> quads = math::to_quads(cubic, 0.25f);

        if (!quads.empty()) {
          path.points.insert(path.points.end(), quads.begin() + 1, quads.end());
        }
#endif
#if 0
        cubic_bezier curve = { m_points[j - 1], m_points[j], m_points[j + 1], m_points[j + 2] };
        cubic_bezier sub_curve = curve;

        const vec2 a = 3 * (-curve.p0 + 3 * curve.p1 - 3 * curve.p2 + curve.p3);
        const vec2 b = 6 * (curve.p0 - 2 * curve.p1 + curve.p2);
        const vec2 c = -3 * (curve.p0 - curve.p1);

        // To get the inflections C'(t) cross C''(t) = at^2 + bt + c = 0 needs to be solved for 't'.
        // The first cooefficient of the quadratic formula is also the denominator.
        const float den = math::cross(b, a);

        float tc = -1.0f;
        float tl = -1.0f;

        if (den != 0.0f) {
          // Two roots might exist, solve with quadratic formula ('tl' is real).
          tc = math::cross(a, c) / den;
          tl = tc * tc + math::cross(b, c) / den;

          // If 'tl < 0' there are two complex roots (no need to solve).
          // If 'tl == 0' there is a real double root at tc (cusp case).
          // If 'tl > 0' two real roots exist at 'tc - Sqrt(tl)' and 'tc + Sqrt(tl)'.
          // if (tl > 0) {
          //   tl = std::sqrtf(tl);
          // }

          if (tl > 0.0) {
            tl = std::sqrtf(tl);
          }

          // if (tl == 0 && tc > math::geometric_epsilon<float> && tc < 1.0 - math::geometric_epsilon<float>) {
          //   const auto& [p, in_p1, in_p2, out_p1, out_p2] = math::split_bezier(curve.p0, curve.p1, curve.p2, curve.p3, tc);

          //   // path.quadratic_to(single_quadratic_approximation({ curve.p0, in_p1, in_p2, p }), p);
          //   // path.quadratic_to(single_quadratic_approximation({ p, out_p1, out_p2, curve.p3 }), curve.p3);
          //   monotonic_cubic_to_quadratics({ curve.p0, in_p1, in_p2, p }, tolerance, path);
          //   monotonic_cubic_to_quadratics({ p, out_p1, out_p2, curve.p3 }, tolerance, path);

          //   console::log("cusppppppppppppppppppppppp");

          //   goto next_segment;
          // }
        } else {
          // One real root might exist, solve linear case ('tl' is NaN).
          tc = -0.5f * math::cross(c, b) / math::cross(c, a);

          // if (tc > math::geometric_epsilon<float> && tc < 1.0 - math::geometric_epsilon<float>) {
          //   const auto& [p, in_p1, in_p2, out_p1, out_p2] = math::split_bezier(curve.p0, curve.p1, curve.p2, curve.p3, tc);

          //   monotonic_cubic_to_quadratics({ curve.p0, in_p1, in_p2, p }, tolerance, path);
          //   monotonic_cubic_to_quadratics({ p, out_p1, out_p2, curve.p3 }, tolerance, path);

          //   console::log("cuspppppppppppppppppppppppsssssssssssssssss");

          //   goto next_segment;
          // }
        }

        const bool tc_in = tc > math::geometric_epsilon<float> && tc < 1.0f - math::geometric_epsilon<float>;
        const bool tl_in = tl > math::geometric_epsilon<float> && tl < 1.0f - math::geometric_epsilon<float>;

#if 0
        if (tc_in && tl_in) {
          monotonic_cubic_to_quadratics(curve, tolerance, path);
          // if (tc > tl) {
          //   std::swap(tc, tl);
          // }

          // const auto& [p, in_p1, in_p2, out_p1, out_p2] = math::split_bezier(curve.p0, curve.p1, curve.p2, curve.p3, tc);

          // monotonic_cubic_to_quadratics({ curve.p0, in_p1, in_p2, p }, tolerance, path);

          // const auto& [p1, in_p1_1, in_p2_1, out_p1_1, out_p2_1] = math::split_bezier(p, out_p1, out_p2, curve.p3, tl * tc);

          // monotonic_cubic_to_quadratics({ p, in_p1_1, in_p2_1, p1 }, tolerance, path);
          // monotonic_cubic_to_quadratics({ p1, out_p1_1, out_p2_1, curve.p3 }, tolerance, path);

          // console::log("cusp 1");
          // console::log(tc);
          // console::log(tl);
        } else if (tc_in) {
          const auto& [p, in_p1, in_p2, out_p1, out_p2] = math::split_bezier(curve.p0, curve.p1, curve.p2, curve.p3, tc);

          monotonic_cubic_to_quadratics({ curve.p0, in_p1, in_p2, p }, tolerance, path);
          monotonic_cubic_to_quadratics({ p, out_p1, out_p2, curve.p3 }, tolerance, path);

          // console::log("cusp 2");
          // console::log(tc);
        } else if (tl_in) {
          const auto& [p, in_p1, in_p2, out_p1, out_p2] = math::split_bezier(curve.p0, curve.p1, curve.p2, curve.p3, tl);

          monotonic_cubic_to_quadratics({ curve.p0, in_p1, in_p2, p }, tolerance, path);
          monotonic_cubic_to_quadratics({ p, out_p1, out_p2, curve.p3 }, tolerance, path);

          // console::log("cusp 3");
          // console::log(tl);
        } else {
#endif
          monotonic_cubic_to_quadratics(curve, tolerance, path);
#if 0
        }
#endif
      //   float t_min = 0.0f;
      //   float t_max = 1.0f;

      //   while (true) {
      //     if (single_quadratic_approximation_error(sub_curve) <= tolerance) {
      //       path.quadratic_to(single_quadratic_approximation(sub_curve), sub_curve.p3);

      //       if (t_max >= 1.0f) {
      //         goto next_segment;
      //       }

      //       t_min = t_max;
      //       t_max = 1.0f;
      //     } else {
      //       t_max = (t_min + t_max) / 2.0f;
      //     }

      //     const auto& [p0, p1, p2, p3] = math::split_bezier(curve.p0, curve.p1, curve.p2, curve.p3, t_min, t_max);

      //     sub_curve = { p0, p1, p2, p3 };
      //   }
#endif
      // next_segment:
        j += 3;
        break;
      }
      }
    }

    return path;
  }

  io::EncodedData& Path::encode(io::EncodedData& data) const {
    if (vacant()) return data.uint32(0);

    data.vector(m_commands);
    data.vector(m_points);
    data.boolean(closed());

    // if (encode_in_out_handles && !closed()) {
    data.vec2(m_in_handle);
    data.vec2(m_out_handle);
    // }

    return data;
  }

  void Path::push_command(const Command command) {
    size_t rem = m_commands_size % 4;

    if (rem == 0) {
      m_commands.push_back(command << 6);
    } else {
      m_commands[m_commands_size / 4] |= command << (6 - rem * 2);
    }

    m_commands_size += 1;
  }

  void Path::insert_command(const Command command, const size_t index) {
    if (index >= m_commands_size) {
      return push_command(command);
    } else if (index == 0) {
      std::vector<Command> commands(m_commands_size + 1);

      for (size_t i = 0; i < m_commands_size; i++) {
        commands[i + 1] = get_command(i);
      }

      commands[0] = Command::Move;
      commands[1] = command;

      m_commands.clear();
      m_commands_size = 0;

      for (size_t i = 0; i < commands.size(); i++) {
        push_command(commands[i]);
      }

      return;
    }

    std::vector<Command> commands_before(index + 1);
    std::vector<Command> commands_after;

    for (size_t i = 0; i < index; i++) {
      commands_before[i] = get_command(i);
    }

    commands_before[index] = command;

    for (size_t i = index; i < m_commands_size; i++) {
      commands_after.push_back(get_command(i));
    }

    m_commands.clear();
    m_commands_size = 0;

    for (size_t i = 0; i < commands_before.size(); i++) {
      push_command(commands_before[i]);
    }

    for (size_t i = 0; i < commands_after.size(); i++) {
      push_command(commands_after[i]);
    }
  }

  void Path::replace_command(const size_t index, const Command command) {
    GK_ASSERT(index < m_commands_size, "Command index out of range.");

    size_t rem = index % 4;

    m_commands[index / 4] &= ~(0b00000011 << (6 - rem * 2));
    m_commands[index / 4] |= command << (6 - rem * 2);
  }

  void Path::remove_command(const size_t index) {
    GK_ASSERT(index < m_commands_size, "Command index out of range.");

    if (index == m_commands_size - 1) {
      size_t rem = (m_commands_size - 1) % 4;

      if (rem == 0) {
        m_commands.pop_back();
      } else {
        m_commands[(m_commands_size - 1) / 4] &= ~(0b00000011 << (6 - rem * 2));
      }

      m_commands_size -= 1;

      return;
    } else if (index == 0) {
      std::vector<Command> commands(m_commands_size - 1);

      for (size_t i = 0; i < m_commands_size - 1; i++) {
        commands[i] = get_command(i + 1);
      }

      commands[0] = Command::Move;

      m_commands.clear();
      m_commands_size = 0;

      for (size_t i = 0; i < commands.size(); i++) {
        push_command(commands[i]);
      }

      return;
    }

    std::vector<Command> commands(m_commands_size - 1);

    for (size_t i = 0; i < index; i++) {
      commands[i] = get_command(i);
    }

    for (size_t i = index + 1; i < m_commands_size; i++) {
      commands[i - 1] = get_command(i);
    }

    m_commands.clear();
    m_commands_size = 0;

    for (size_t i = 0; i < commands.size(); i++) {
      push_command(commands[i]);
    }
  }

}
