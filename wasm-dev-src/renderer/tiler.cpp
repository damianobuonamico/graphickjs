#include "tiler.h"

#include "geometry/path.h"
#include "geometry/contour.h"

#include "../math/mat2x3.h"
#include "../math/matrix.h"
#include "../math/math.h"

#include "../utils/console.h"

#include <algorithm>

// TODO: zoom and transform operations should use doubles
// TODO: fix right border of tiger (near min_y)

namespace Graphick::Renderer {

#define OPAQUE_AND_MASKED 0
#define SAMPLE_COUNT 16

  static constexpr float tolerance = 0.25f;

  static inline ivec2 tile_coords(const vec2 p) {
    return { (int)std::floor(p.x / TILE_SIZE), (int)std::floor(p.y / TILE_SIZE) };
  }

  static inline ivec2 tile_coords_clamp(const vec2 p, const ivec2 tiles_count) {
    return { std::clamp((int)std::floor(p.x / TILE_SIZE), 0, tiles_count.x - 1), std::clamp((int)std::floor(p.y / TILE_SIZE), 0, tiles_count.y - 1) };
  }

  static inline int tile_index(const ivec2 coords, const ivec2 tiles_count) {
    return coords.x + coords.y * tiles_count.x;
  }

  static inline int tile_index(const int16_t tile_x, const int16_t tile_y, const int16_t tiles_count_x) {
    return tile_x + tile_y * tiles_count_x;
  }

  static inline float x_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (x3 - x4) -
      (x1 - x2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
  }

  static inline float x_intersect(float one_over_m, float q, float y) {
    return (y - q) * one_over_m;
  }

  static inline float y_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float num = (x1 * y2 - y1 * x2) * (y3 - y4) -
      (y1 - y2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
  }

  static inline float y_intersect(float m, float q, float x) {
    return m * x + q;
  }

  static void clip_to_left(std::vector<vec2>& points, float x) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x < x) {
        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static void clip_to_right(std::vector<vec2>& points, float x) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.x > x) {
        if (points[i + 1].x < x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].x > x) {
          new_points.push_back({ x, y_intersect(x, -1000.0f, x, 1000.0f, point.x, point.y, points[i + 1].x, points[i + 1].y) });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static void clip_to_top(std::vector<vec2>& points, float y) {
    if (points.empty()) return;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y < y) {
        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      } else {
        new_points.push_back(point);

        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
    }

    points = new_points;
  }

  static vec2 clip_to_bottom(std::vector<vec2>& points, float y) {
    vec2 min = std::numeric_limits<vec2>::max();

    if (points.empty()) return min;
    std::vector<vec2> new_points;

    for (int i = 0; i < points.size() - 1; i++) {
      vec2 point = points[i];

      if (point.y > y) {
        if (points[i + 1].y < y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
          min = Math::min(min, new_points.back());
        }
      } else {
        new_points.push_back(point);
        min = Math::min(min, new_points.back());

        if (points[i + 1].y > y) {
          new_points.push_back({ x_intersect(-1000.0f, y, 1000.0f, y, point.x, point.y, points[i + 1].x, points[i + 1].y), y });
          min = Math::min(min, new_points.back());
        }
      }
    }

    if (new_points.size() > 2 && new_points.front() != new_points.back()) {
      new_points.push_back(new_points.front());
      min = Math::min(min, new_points.back());
    }

    points = new_points;

    return min;
  }

  static vec2 clip(std::vector<vec2>& points, rect visible) {
    clip_to_left(points, visible.min.x);
    clip_to_right(points, visible.max.x);
    clip_to_top(points, visible.min.y);

    return clip_to_bottom(points, visible.max.y);
  }

  inline static int16_t sign(float x) {
    return (0 < x) - (x < 0);
  }

  /* -- DrawableTiler -- */

  struct TempTile {
    std::vector<uvec4> segments;
    float cover_table[TILE_SIZE];
    int8_t sign;

    ~TempTile() = default;
  };

  static std::vector<std::unique_ptr<TempTile>> temp_tiles;

  DrawableTiler::DrawableTiler(const Drawable& drawable, const rect& visible, const float zoom, const ivec2 position, const vec2 subpixel, const ivec2 tiles_count) {
    rect bounds = {
      Math::floor((drawable.bounds.min - subpixel - 1.0f) / TILE_SIZE) * TILE_SIZE,
      Math::ceil((drawable.bounds.max - subpixel + 1.0f) / TILE_SIZE) * TILE_SIZE
    };

    ivec2 min_coords = tile_coords(bounds.min) + position;
    ivec2 max_coords = tile_coords(bounds.max) + position;

    bounds += subpixel;

    m_offset = min_coords;
    m_size = max_coords - min_coords;

    temp_tiles.clear();
    temp_tiles.resize(m_size.x * m_size.y);

    for (const Geometry::Contour& contour : drawable.contours) {
      if (contour.points.size() < 2) continue;

      move_to(contour.points.front() - bounds.min);

      for (size_t i = 1; i < contour.points.size(); i++) {
        line_to(contour.points[i] - bounds.min);
      }
    }

    pack(drawable.paint.rule, tiles_count);
  }

  void DrawableTiler::move_to(const vec2 p0) {
    m_p0 = p0;
  }

  static constexpr float max_over_tile_size = 255.0f / (float)TILE_SIZE;
  static constexpr float tile_size_over_max = (float)TILE_SIZE / 255.0f;

  void DrawableTiler::line_to(const vec2 p3) {
    if (Math::is_almost_equal(m_p0, p3)) return;

    vec2 p0 = m_p0;
    vec2 vec = p3 - p0;

    int16_t x_dir = sign(vec.x);
    int16_t y_dir = sign(vec.y);
    int16_t x = (int16_t)std::floor(p0.x);
    int16_t y = (int16_t)std::floor(p0.y);
    int16_t x_tile_dir = x_dir * TILE_SIZE;
    int16_t y_tile_dir = y_dir * TILE_SIZE;
    int16_t tile_x = x / TILE_SIZE;
    int16_t tile_y = y / TILE_SIZE;

    m_p0 = p3;
    m_tile_y_prev = tile_y;

    float row_t1 = std::numeric_limits<float>::infinity();
    float col_t1 = std::numeric_limits<float>::infinity();
    float dtdx = (float)TILE_SIZE / (vec.x);
    float dtdy = (float)TILE_SIZE / (vec.y);

    if (p0.y != p3.y) {
      float next_y = (float)(tile_y + (p3.y > p0.y ? 1 : 0)) * TILE_SIZE;
      row_t1 = std::min(1.0f, (next_y - p0.y) / vec.y);
    }

    if (p0.x != p3.x) {
      float next_x = (float)(tile_x + (p3.x > p0.x ? 1 : 0)) * TILE_SIZE;
      col_t1 = std::min(1.0f, (next_x - p0.x) / vec.x);
    }

    vec2 step = { std::abs(dtdx), std::abs(dtdy) };
    vec2 from = p0;


    while (true) {
      float t1 = std::min(row_t1, col_t1);

      vec2 to = p0 + vec * t1;
      vec2 tile_pos = TILE_SIZE * vec2{ (float)tile_x, (float)tile_y };
      vec2 from_delta = from - tile_pos;
      vec2 to_delta = to - tile_pos;

      int index = tile_index(tile_x, tile_y, m_size.x);

      if (temp_tiles[index].get() == nullptr) {
        temp_tiles[index] = std::make_unique<TempTile>();
        temp_tiles[index]->segments.reserve(25);
      }

      if (from_delta.y != to_delta.y) {
        uint8_t y0 = static_cast<uint8_t>(std::roundf(from_delta.y * max_over_tile_size));
        uint8_t y1 = static_cast<uint8_t>(std::roundf(to_delta.y * max_over_tile_size));

        if (y0 != y1) {
          uint8_t x0 = static_cast<uint8_t>(std::roundf(from_delta.x * max_over_tile_size));
          uint8_t x1 = static_cast<uint8_t>(std::roundf(to_delta.x * max_over_tile_size));

          float cover, fy0, fy1;

          if (y0 < y1) {
            fy0 = (float)y0 * tile_size_over_max;
            fy1 = (float)y1 * tile_size_over_max;
            cover = 1.0f;
          } else {
            fy0 = (float)y1 * tile_size_over_max;
            fy1 = (float)y0 * tile_size_over_max;
            cover = -1.0f;
          }

          float iy0 = std::floorf(fy0);
          float iy1 = std::ceilf(fy1);

          int i0 = (int)iy0;
          int i1 = (int)iy1;

          temp_tiles[index]->cover_table[i0] += cover * (iy0 + 1.0f - fy0);

          for (int j = i0 + 1; j < i1; j++) {
            temp_tiles[index]->cover_table[j] += cover * (1.0f);
          }

          temp_tiles[index]->cover_table[i1 - 1] -= cover * (iy1 - fy1);

          temp_tiles[index]->segments.emplace_back(x0, y0, x1, y1);
        }
      }

      bool fuzzy_equal;

      if (row_t1 < col_t1) {
        fuzzy_equal = row_t1 >= 1.0f - 0.0001f;
        row_t1 = std::min(1.0f, row_t1 + step.y);

        y += y_tile_dir;
        tile_y += y_dir;
      } else {
        fuzzy_equal = col_t1 >= 1.0f - 0.0001f;
        col_t1 = std::min(1.0f, col_t1 + step.x);
        x += x_tile_dir;
        tile_x += x_dir;
      }

      if (fuzzy_equal) {
        x = (int16_t)std::floorf(p3.x);
        y = (int16_t)std::floorf(p3.y);

        tile_x = x / TILE_SIZE;
        tile_y = y / TILE_SIZE;
      }

      if (tile_y != m_tile_y_prev) {
        int sign_index = tile_index(tile_x, std::min(tile_y, m_tile_y_prev), m_size.x);

        if (temp_tiles[sign_index].get() == nullptr) {
          temp_tiles[sign_index] = std::make_unique<TempTile>();
        }

        temp_tiles[sign_index]->sign += (int8_t)(tile_y - m_tile_y_prev);
        m_tile_y_prev = tile_y;
      }

      from = to;

      if (fuzzy_equal) break;
    }
  }

  void DrawableTiler::pack(const FillRule rule, const ivec2 tiles_count) {
    float cover_table[TILE_SIZE] = { 0.0f };
    int winding = 0;

    for (int16_t y = 0; y < m_size.y; y++) {
      std::memset(cover_table, 0, TILE_SIZE * sizeof(float));
      winding = 0;

      for (int16_t x = 0; x < m_size.x; x++) {
        int index = tile_index(x, y, m_size.x);

        if (temp_tiles[index].get() != nullptr) {
          auto& mask = m_masks[index];

          std::memcpy(mask.cover_table, cover_table, TILE_SIZE * sizeof(float));
          winding += temp_tiles[index]->sign;

          if (temp_tiles[index]->segments.empty()) {
            continue;
          }

          mask.segments = std::move(temp_tiles[index]->segments);

          for (int i = 0; i < TILE_SIZE; i++) {
            cover_table[i] += temp_tiles[index]->cover_table[i];
          }
        } else if (
          (rule == FillRule::NonZero && winding != 0) ||
          (rule == FillRule::EvenOdd && winding % 2 != 0)
          ) {
          m_spans.push_back(Span{ x, y, 1 });
        }
      }
    }
  }

  /* -- Tiler -- */

  Tiler::Tiler() {
    m_segments = new uint8_t[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE * 4];
    m_cover_table = new float[SEGMENTS_TEXTURE_SIZE * SEGMENTS_TEXTURE_SIZE];
  }

  Tiler::~Tiler() {
    delete[] m_segments;
    delete[] m_cover_table;
  }

  void Tiler::reset(const Viewport& viewport) {
    m_tiles_count = { (int)(std::ceil((float)viewport.size.x / TILE_SIZE)) + 2, (int)(std::ceil((float)viewport.size.y / TILE_SIZE)) + 2 };
    m_position = {
      (int)(viewport.position.x > 0 ? std::floor(viewport.position.x * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.x * viewport.zoom / TILE_SIZE)),
      (int)(viewport.position.y > 0 ? std::floor(viewport.position.y * viewport.zoom / TILE_SIZE) : std::ceil(viewport.position.y * viewport.zoom / TILE_SIZE))
    };
    vec2 offset = {
      std::fmodf(viewport.position.x * viewport.zoom, TILE_SIZE),
      std::fmodf(viewport.position.y * viewport.zoom, TILE_SIZE)
    };
    m_subpixel = (viewport.position * viewport.zoom) % TILE_SIZE - offset;
    m_zoom = viewport.zoom;
    m_visible = { -viewport.position, vec2{(float)viewport.size.x / viewport.zoom, (float)viewport.size.y / viewport.zoom} - viewport.position };

    m_opaque_tiles.clear();
    m_masked_tiles.clear();

    m_segments_ptr = m_segments;
    m_cover_table_ptr = m_cover_table;

    m_culled_tiles = std::vector<bool>(m_tiles_count.x * m_tiles_count.y, false);
  }

  struct QuadraticSolutions {
    uint8_t count;
    double solutions[2];

    QuadraticSolutions() : count(0), solutions{ 0.0f, 0.0f } {}
    QuadraticSolutions(const double x) : count(1), solutions{ x, 0.0f } {}
    QuadraticSolutions(const double x1, const double x2) : count(2), solutions{ x1, x2 } {}
  };

  struct CubicSolutions {
    uint8_t count;
    double solutions[3];

    CubicSolutions() : count(0), solutions{ 0.0f, 0.0f, 0.0f } {}
    CubicSolutions(const double x) : count(1), solutions{ x, 0.0f, 0.0f } {}
    CubicSolutions(const double x1, const double x2) : count(2), solutions{ x1, x2, 0.0f } {}
    CubicSolutions(const double x1, const double x2, const double x3) : count(3), solutions{ x1, x2, x3 } {}
    CubicSolutions(const QuadraticSolutions& quadratic) : count(quadratic.count), solutions{ quadratic.solutions[0], quadratic.solutions[1], 0.0f } {}
  };


  static inline double solve_linear(const double a, const double b) {
    /* Assuming a != 0 */
    return -b / a;
  }

  static QuadraticSolutions solve_quadratic(double a, double b, double c) {
    if (Math::is_almost_zero(a)) {
      /* It is a linear equation */

      return { solve_linear(b, c) };
    }

    double discriminant = b * b - 4.0 * a * c;

    if (Math::is_almost_zero(discriminant)) {
      /* One real root. */

      double root = -b / (2.0 * a);

      // TODO: ask if roots with multiplicity > 1 should be considered as separate roots
      return { root, root };
    } else if (discriminant < 0.0) {
      /* No real roots. */

      return {};
    }

    /* Two real roots. */

    double q = std::sqrt(discriminant);
    double a2 = 2.0 * a;

    return { (q - b) / a2, (-b - q) / a2 };
  }

  static CubicSolutions solve_cubic(double a, double b, double c, double d) {
    if (Math::is_almost_zero(a)) {
      /* It is a quadratic equation */

      return solve_quadratic(b, c, d);
    }

    if (Math::is_almost_zero(d)) {
      /* One root is 0. */

      CubicSolutions solutions = solve_quadratic(a, b, c);
      solutions.count++;
      return solutions;
    }

    double p = (3 * a * c - b * b) / (3 * a * a);
    /* Calculate coefficients of the depressed cubic equation: y^3 + py + q = 0 */
    // double q = (2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);
    double q = (2 * b * b / a - 9 * c + 27 * a * d / b) / (27 * a * a / b);

    double discriminant = (q * q) / 4 + (p * p * p) / 27;
    /* Calculate discriminant */

    if (Math::is_almost_zero(discriminant)) {
      double u = std::cbrt(-q / 2);
      /* Three real roots, two of them are equal */
      double realRoot1 = 2 * u - b / (3 * a);
      double realRoot2 = -u - b / (3 * a);

      // TODO: ask if roots with multiplicity > 1 should be considered as different roots
      return { realRoot1, realRoot2, realRoot2 };
    } else if (discriminant > 0) {
      double u = std::cbrt(-q / 2 + std::sqrt(discriminant));
      /* One real root and two complex roots */
      double v = std::cbrt(-q / 2 - std::sqrt(discriminant));
      double realRoot = u + v - b / (3 * a);

      return { realRoot };
    } else {
      double phi = std::acos(-q / 2 * std::sqrt(-27 / (p * p * p)));
      double b1 = -b / (3.0 * a);
      double xi = 2.0 * std::sqrt(-p / 3);
      /* Three distinct real roots */
      double root1 = xi * std::cos(phi / 3) + b1;
      double root2 = xi * std::cos((phi + 2 * MATH_PI) / 3) + b1;
      double root3 = xi * std::cos((phi + 4 * MATH_PI) / 3) + b1;

      return { root1, root2, root3 };
    }
  }

  struct dvec2 {
    double x;
    double y;

    double operator[] (const int i) const {
      return i == 0 ? x : y;
    }

    dvec2 operator* (const double s) const {
      return { x * s, y * s };
    }

    dvec2 operator+ (const dvec2& v) const {
      return { x + v.x, y + v.y };
    }

    dvec2 operator+(const double s) const {
      return { x + s, y + s };
    }

    dvec2 operator- (const dvec2& v) const {
      return { x - v.x, y - v.y };
    }

    dvec2 operator- (const double s) const {
      return { x - s, y - s };
    }

    dvec2 operator- () const {
      return { -x, -y };
    }
  };

  dvec2 operator* (const double s, const dvec2& v) {
    return { v.x * s, v.y * s };
  }

  static std::vector<vec2> line_rect_intersection_points(const vec2 p0, const vec2 p3, const rect& rect) {
    std::vector<vec2> intersection_points;
    std::vector<double> intersections;

    dvec2 dp0 = { p0.x, p0.y };
    dvec2 dp3 = { p3.x, p3.y };

    dvec2 a = dp3 - dp0;

    double t1 = solve_linear(a.x, dp0.x - (double)rect.min.x);
    double t2 = solve_linear(a.x, dp0.x - (double)rect.max.x);
    double t3 = solve_linear(a.y, dp0.y - (double)rect.min.y);
    double t4 = solve_linear(a.y, dp0.y - (double)rect.max.y);

    if (t1 >= 0.0 && t1 <= 1.0) intersections.push_back(t1);
    if (t2 >= 0.0 && t2 <= 1.0) intersections.push_back(t2);
    if (t3 >= 0.0 && t3 <= 1.0) intersections.push_back(t3);
    if (t4 >= 0.0 && t4 <= 1.0) intersections.push_back(t4);

    if (intersections.empty()) return intersection_points;

    std::sort(intersections.begin(), intersections.end());

    for (double t : intersections) {
      dvec2 p = dp0 + (dp3 - dp0) * t;

      if (Math::is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersection_points.push_back({ (float)p.x, (float)p.y });
      }
    }

    return intersection_points;
  }

  static std::vector<vec3> bezier_rect_intersection_points(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3, const rect& rect) {
    std::vector<vec3> intersection_points;
    std::vector<double> intersections;

    dvec2 dp0 = { p0.x, p0.y };
    dvec2 dp1 = { p1.x, p1.y };
    dvec2 dp2 = { p2.x, p2.y };
    dvec2 dp3 = { p3.x, p3.y };

    dvec2 a = -dp0 + 3.0 * dp1 - 3.0 * dp2 + dp3;
    dvec2 b = 3.0 * dp0 - 6.0 * dp1 + 3.0 * dp2;
    dvec2 c = -3.0 * dp0 + 3.0 * dp1;

    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        CubicSolutions roots = solve_cubic(a[k], b[k], c[k], dp0[k] - rect[j][k]);

        for (uint8_t i = 0; i < roots.count; i++) {
          double t = roots.solutions[i];

          if (t >= 0.0 && t <= 1.0) intersections.push_back(t);
        }
      }
    }

    if (intersections.empty()) return intersection_points;

    std::sort(intersections.begin(), intersections.end());


    for (double t : intersections) {
      double t_sq = t * t;
      dvec2 p = a * t_sq * t + b * t_sq + c * t + dp0;

      if (Math::is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersection_points.push_back({ (float)t, (float)p.x, (float)p.y });
      }
    }

    // intersection_points.erase(std::unique(intersection_points.begin(), intersection_points.end(), [](const vec3& l, const vec3& r) { return std::abs(l.y - r.y) < GEOMETRY_CURVE_ERROR && std::abs(l.z - r.z) < GEOMETRY_CURVE_ERROR; }), intersection_points.end());

    return intersection_points;
  }

  struct Segment {
    vec2 p0;
    vec2 p1;
    vec2 p2;
    vec2 p3;

    bool is_linear;

    Segment(vec2 p0, vec2 p3) : p0(p0), p3(p3), is_linear(true) {}
    Segment(vec2 p0, vec2 p1, vec2 p2, vec2 p3) : p0(p0), p1(p1), p2(p2), p3(p3), is_linear(false) {}
  };

  enum Bound {
    BoundTop,
    BoundRight,
    BoundBottom,
    BoundLeft,
    BoundNone
  };

  static std::vector<std::pair<vec2, Bound>> line_rect_intersection_points_bound(const vec2 p0, const vec2 p3, const rect& rect) {
    std::vector<std::pair<vec2, Bound>> intersection_points;
    std::vector<std::pair<double, Bound>> intersections;

    dvec2 dp0 = { p0.x, p0.y };
    dvec2 dp3 = { p3.x, p3.y };

    dvec2 a = dp3 - dp0;

    double t1 = solve_linear(a.x, dp0.x - (double)rect.min.x);
    double t2 = solve_linear(a.x, dp0.x - (double)rect.max.x);
    double t3 = solve_linear(a.y, dp0.y - (double)rect.min.y);
    double t4 = solve_linear(a.y, dp0.y - (double)rect.max.y);

    if (t1 >= 0.0 && t1 <= 1.0) intersections.emplace_back(t1, Bound::BoundLeft);
    if (t2 >= 0.0 && t2 <= 1.0) intersections.emplace_back(t2, Bound::BoundRight);
    if (t3 >= 0.0 && t3 <= 1.0) intersections.emplace_back(t3, Bound::BoundTop);
    if (t4 >= 0.0 && t4 <= 1.0) intersections.emplace_back(t4, Bound::BoundBottom);

    if (intersections.empty()) return intersection_points;

    std::sort(intersections.begin(), intersections.end());

    for (auto& [t, bound] : intersections) {
      dvec2 p = dp0 + (dp3 - dp0) * t;

      if (Math::is_point_in_rect({ (float)p.x, (float)p.y }, rect, GK_POINT_EPSILON)) {
        intersection_points.emplace_back(vec2{ (float)p.x, (float)p.y }, bound);
      }
    }

    return intersection_points;
  }


  static Drawable clip_drawable(const Drawable& drawable, const rect& clip) {
    Drawable clipped(0, drawable.paint, { Math::max(drawable.bounds.min, clip.min), Math::min(drawable.bounds.max, clip.max) });

    for (auto& contour : drawable.contours) {
      Geometry::Contour& clipped_contour = clipped.contours.emplace_back();

      std::vector<vec2> new_points = contour.points;

      clip_to_left(new_points, clip.min.x);
      clip_to_top(new_points, clip.min.y);
      clip_to_bottom(new_points, clip.max.y);
      clip_to_right(new_points, clip.max.x);

      clipped_contour.points = new_points;

      clipped_contour.close();
    }

    return clipped;
  }

  void Tiler::process_drawable(const Drawable& drawable, const rect& visible, const vec2 offset, const bool clip) {
    ivec2 tile_offset = tile_coords(offset);
    vec2 pixel_offset = offset - TILE_SIZE * vec2{ (float)tile_offset.x, (float)tile_offset.y };

    DrawableTiler tiler(clip ? clip_drawable(drawable, rect{ { -32.0f, -32.0f }, (visible.max - visible.min) * m_zoom + 32.0f }) : drawable, visible, m_zoom, m_position + tile_offset, m_subpixel - pixel_offset, m_tiles_count);

    const std::unordered_map<int, DrawableTiler::Mask>& masks = tiler.masks();
    const std::vector<DrawableTiler::Span>& spans = tiler.spans();
    const ivec2 tiler_offset = tiler.offset();
    const ivec2 size = tiler.size();

    for (const auto& [index, mask] : masks) {
      ivec2 coords = {
        index % size.x + tiler_offset.x + 1,
        index / size.x + tiler_offset.y + 1
      };

      if (coords.x < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int absolute_index = tile_index(coords, m_tiles_count);
      if (m_culled_tiles[absolute_index]) continue;

      int offset = (int)(m_segments_ptr - m_segments) / 4;
      int cover_offset = (int)(m_cover_table_ptr - m_cover_table);

      uint32_t segments_size = (uint32_t)mask.segments.size();

      m_segments_ptr[0] = (uint8_t)segments_size;
      m_segments_ptr[1] = (uint8_t)(segments_size >> 8);
      m_segments_ptr[2] = (uint8_t)(segments_size >> 16);
      m_segments_ptr[3] = (uint8_t)(segments_size >> 24);
      m_segments_ptr += 4;

      for (auto segment : mask.segments) {
        m_segments_ptr[0] = segment.x0;
        m_segments_ptr[1] = segment.y0;
        m_segments_ptr[2] = segment.x1;
        m_segments_ptr[3] = segment.y1;
        m_segments_ptr += 4;
      }

      // TODO: bounds check
      memcpy(m_cover_table_ptr, &mask.cover_table, TILE_SIZE * sizeof(float));
      m_cover_table_ptr += TILE_SIZE;

      m_masked_tiles.push_back(MaskedTile{
        drawable.paint.color,
        absolute_index,
        { (uint16_t)(offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(offset / SEGMENTS_TEXTURE_SIZE) },
        { (uint16_t)(cover_offset % SEGMENTS_TEXTURE_SIZE), (uint16_t)(cover_offset / SEGMENTS_TEXTURE_SIZE) },
        drawable.paint.z_index
        });
    }

    for (auto& span : spans) {
      ivec2 coords = { span.tile_x + tiler_offset.x + 1, span.tile_y + tiler_offset.y + 1 };
      if (coords.x + span.width < 0 || coords.y < 0 || coords.x >= m_tiles_count.x || coords.y >= m_tiles_count.y) continue;

      int16_t width = coords.x < 0 ? span.width + coords.x : span.width;
      coords.x = std::max(coords.x, 0);

      for (int i = 0; i < width; i++) {
        if (coords.x + i >= m_tiles_count.x) break;

        int index = tile_index({ coords.x + i, coords.y }, m_tiles_count);
        if (!m_culled_tiles[index]/*&& color.a == 1.0f*/) {
          m_opaque_tiles.push_back({ drawable.paint.color, index, drawable.paint.z_index });
          m_culled_tiles[index] = true;
        }
      }
    }
  }

  void Tiler::process_stroke(const Geometry::Path& path, const mat2x3& transform, const Stroke& stroke) {
    GK_TOTAL("Tiler::process_stroke");
    const float radius = 0.5f * stroke.width * m_zoom;

    rect path_rect = transform * path.bounding_rect();

    path_rect.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    path_rect.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();
    if (overlap <= 0.0f) return;

    const mat2x3 transform_zoom = transform * m_zoom;
    const auto& segments = path.segments();

    const bool clip_drawable = stroke.width > std::min(m_visible.size().x, m_visible.size().y);

    // TODO: test and tweak overlap threshold
    if (overlap > 0.7f) {
      Drawable drawable(path.closed() ? 2 : 1, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, (path_rect - m_visible.min) * m_zoom);
      Geometry::Contour* contour = &drawable.contours.front();

      contour->begin((transform * segments.front().p0() - m_visible.min) * m_zoom, false);

      for (size_t i = 0; i < segments.size(); i++) {
        auto& raw_segment = segments.at(i);

        if (raw_segment.is_linear()) {
          // TODO: test const reference vs copy
          contour->offset_segment((transform * raw_segment.p3() - m_visible.min) * m_zoom, radius);
        } else {
          contour->offset_segment(
            (transform * raw_segment.p1() - m_visible.min) * m_zoom,
            (transform * raw_segment.p2() - m_visible.min) * m_zoom,
            (transform * raw_segment.p3() - m_visible.min) * m_zoom,
            radius
          );
        }
      }

      if (path.closed()) {
        contour->close();
        contour = &drawable.contours.back();
      }

      contour->begin((transform * segments.back().p3() - m_visible.min) * m_zoom, false);

      for (int i = static_cast<int>(segments.size()) - 1; i >= 0; i--) {
        auto& raw_segment = segments.at(i);

        if (raw_segment.is_linear()) {
          contour->offset_segment(
            (transform * raw_segment.p0() - m_visible.min) * m_zoom,
            radius
          );
        } else {
          contour->offset_segment(
            (transform * raw_segment.p2() - m_visible.min) * m_zoom,
            (transform * raw_segment.p1() - m_visible.min) * m_zoom,
            (transform * raw_segment.p0() - m_visible.min) * m_zoom,
            radius
          );
        }
      }

      contour->close();

      return process_drawable(drawable, m_visible, m_visible.min * m_zoom, clip_drawable);
    }

    size_t len = segments.size();
    size_t i = 0;

    std::vector<std::vector<Segment>> clipped_contours(1);

    // TODO: if only one clipped contour, close it

    rect visible = m_visible;

    visible.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    visible.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    visible.min -= 32.0f / m_zoom;
    visible.max += 32.0f / m_zoom;

    for (size_t i = 0; i < len; i++) {
      auto& raw_segment = segments.at(i);

      vec2 p0 = transform * raw_segment.p0() - visible.min;
      vec2 p3 = transform * raw_segment.p3() - visible.min;

      bool p0_in = Math::is_point_in_rect(p0, visible - visible.min);
      bool p3_in = Math::is_point_in_rect(p3, visible - visible.min);

      if (raw_segment.is_linear()) {

        if (p0_in && p3_in) {
          /* Entire segment is visible. */

          clipped_contours.back().emplace_back(p0, p3);
          continue;
        }

        std::vector<vec2> intersections = line_rect_intersection_points(p0, p3, visible - visible.min);

        if (intersections.empty()) continue;

        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {
            if (p0_in) {
              clipped_contours.back().emplace_back((k < 1 ? p0 : intersections[k - 1]), intersections[k]);
            } else {
              if (!clipped_contours.back().empty()) clipped_contours.emplace_back();
              clipped_contours.back().emplace_back(intersections[k], (k >= intersections.size() - 1 ? p3 : intersections[k + 1]));
            }
          }
        }
      } else {
        vec2 p1 = transform * raw_segment.p1() - visible.min;
        vec2 p2 = transform * raw_segment.p2() - visible.min;

        // TODO: check if all inside or all outside with control points
        std::vector<vec3> intersections = bezier_rect_intersection_points(p0, p1, p2, p3, visible - visible.min);

        if (intersections.empty()) {
          if (p0_in) {
            /* Entire segment is visible. */

            clipped_contours.back().emplace_back(p0, p1, p2, p3);
          }

          /* Segment is completely outside. */

          continue;
        }

        for (int k = 0; k < (int)intersections.size(); k++) {
          if (k % 2 == 0) {
            if (p0_in) {
              auto segment = Math::split_bezier(
                p0,
                p1,
                p2,
                p3,
                (k < 1 ? 0.0f : intersections[k - 1].x),
                intersections[k].x
              );

              clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
            } else {
              if (!clipped_contours.back().empty()) clipped_contours.emplace_back();

              auto segment = Math::split_bezier(
                p0,
                p1,
                p2,
                p3,
                intersections[k].x,
                (k >= intersections.size() - 1 ? 1.0f : intersections[k + 1].x)
              );

              clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
            }
          } else if (p3_in && k == (int)intersections.size() - 1) {
            if (!clipped_contours.back().empty()) clipped_contours.emplace_back();

            auto segment = Math::split_bezier(
              p0,
              p1,
              p2,
              p3,
              intersections[k].x,
              (k >= intersections.size() - 1 ? 1.0f : intersections[k + 1].x)
            );

            clipped_contours.back().emplace_back(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
          } else {
            if (!clipped_contours.back().empty()) clipped_contours.emplace_back();
          }
        }
      }
    }

    Drawable drawable(0, Paint{ stroke.color, FillRule::NonZero, stroke.z_index }, rect{});

    for (auto& clipped_contour : clipped_contours) {
      if (clipped_contour.empty()) continue;

      Geometry::Contour* contour = &drawable.contours.emplace_back();

      vec2 p0 = clipped_contour.front().p0 * m_zoom;

      contour->begin(p0, false);

      Math::min(drawable.bounds.min, p0, drawable.bounds.min);
      Math::max(drawable.bounds.max, p0, drawable.bounds.max);

      for (size_t i = 0; i < clipped_contour.size(); i++) {
        auto& segment = clipped_contour.at(i);

        vec2 p3 = segment.p3 * m_zoom;

        if (segment.is_linear) {
          // TODO: test const reference vs copy
          contour->offset_segment(p3, radius);
        } else {
          vec2 p1 = segment.p1 * m_zoom;
          vec2 p2 = segment.p2 * m_zoom;

          contour->offset_segment(
            p1,
            p2,
            p3,
            radius
          );

          // TODO: optimize away this operations (perform intersection of bounds with visible rect)
          Math::min(drawable.bounds.min, p1, drawable.bounds.min);
          Math::max(drawable.bounds.max, p1, drawable.bounds.max);

          Math::min(drawable.bounds.min, p2, drawable.bounds.min);
          Math::max(drawable.bounds.max, p2, drawable.bounds.max);
        }

        Math::min(drawable.bounds.min, p3, drawable.bounds.min);
        Math::max(drawable.bounds.max, p3, drawable.bounds.max);
      }

      contour->begin(clipped_contour.back().p3 * m_zoom, false);

      for (int i = static_cast<int>(clipped_contour.size()) - 1; i >= 0; i--) {
        auto& segment = clipped_contour.at(i);

        vec2 p0 = segment.p0 * m_zoom;

        if (segment.is_linear) {
          contour->offset_segment(p0, radius);
        } else {
          vec2 p1 = segment.p1 * m_zoom;
          vec2 p2 = segment.p2 * m_zoom;

          contour->offset_segment(
            p2,
            p1,
            p0,
            radius
          );
        }
      }

      contour->close();
    }

    // drawable.bounds.min -= 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    // drawable.bounds.max += 1.1f * 0.5f * stroke.width * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    drawable.bounds.min -= 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);
    drawable.bounds.max += 1.1f * radius * (stroke.join == LineJoin::Miter ? stroke.miter_limit : 1.0f);

    // drawable.bounds *= m_zoom;

    process_drawable(drawable, visible, visible.min * m_zoom, clip_drawable);
  }

  void Tiler::process_fill(const Geometry::Path& path, const mat2x3& transform, const Fill& fill) {
    GK_TOTAL("Tiler::process_fill");

    const rect path_rect = transform * path.bounding_rect();

    const float overlap = Math::rect_rect_intersection_area(path_rect, m_visible) / path_rect.area();
    if (overlap <= 0.0f) return;

    Drawable drawable(1, fill, (path_rect - m_visible.min) * m_zoom);
    Geometry::Contour& contour = drawable.contours.front();

    const auto& segments = path.segments();
    const vec2 first = (transform * segments.front().p0() - m_visible.min) * m_zoom;

    contour.begin(first);

    for (size_t i = 0; i < segments.size(); i++) {
      auto& raw_segment = segments.at(i);

      if (raw_segment.is_linear()) {
        // TODO: test const reference vs copy
        contour.push_segment((transform * raw_segment.p3() - m_visible.min) * m_zoom);
      } else {
        contour.push_segment(
          (transform * raw_segment.p1() - m_visible.min) * m_zoom,
          (transform * raw_segment.p2() - m_visible.min) * m_zoom,
          (transform * raw_segment.p3() - m_visible.min) * m_zoom
        );
      }
    }

    contour.close();

    process_drawable(drawable, m_visible, m_visible.min * m_zoom, overlap < 0.7f);
  }

}
