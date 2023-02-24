#include "freehand_entity.h"

void FreehandEntity::add_point(const vec2& position, float pressure, double time) {
  vec2 smoothed_position = WobbleSmoother::update(m_position + position, time);

  if (m_points.size() > 1) {
    m_points[m_points.size() - 1].position = smoothed_position;
  }

  m_points.push_back({ m_position + position, pressure });
}

void FreehandEntity::render(float zoom) const {
  std::vector<FreehandPathPoint> points = simplify_first ? simplify_path(smooth_freehand_path(m_points, 4), simplification_tolerance, true) : smooth_freehand_path(m_points, 4);

  if (m_vertices.size() > 10) {
    console::log(m_vertices.size());
  }
  std::vector<VertexEntity*> vertices{};

  std::vector<uint> corners = detect_corners(points, min_radius, max_radius, max_iterations, min_angle);

  vec4 curve_color{ 0.0f, 0.0f, 1.0f, 1.0f };
  m_curves.clear();

  if (points.size() > 1) {
    for (size_t i = 0; i < corners.size() - 1; i++) {
      size_t start = corners[i];
      size_t end = corners[i + 1];

      auto curves = fit_to_bezier_curves(points, start, end, max_fit_error);

      for (auto curve : curves) {
        if (vertices.empty()) {
          vertices.push_back(new VertexEntity{ curve.p0, curve.p1 - curve.p0, false });
        } else {
          vertices[vertices.size() - 1]->set_right(curve.p1 - curve.p0);
        }

        vertices.push_back(new VertexEntity{ curve.p3, curve.p2 - curve.p3, true });
      }
    }
  }

  if (vertices.size() < 2) return;

  VertexEntity* last_vertex = nullptr;

  for (VertexEntity* vertex : vertices) {
    if (last_vertex) {
      m_curves.push_back(BezierEntity{ *last_vertex, *vertex, nullptr });
    }

    last_vertex = vertex;
  }

  console::log(m_curves.size());

  if (m_curves.empty()) {
    return;
  }

  Geometry geo{};

  TessellationParams params = {
    m_transform.position().get(), zoom, MATH_PI / 80.0f,
    5.0f, vec4(0.5f, 0.5f, 0.5f, 1.0f),
    JoinType::Round, CapType::Round, 10.0f,
    false, false, true, false, true,
    { vec2{}, vec2{}, 0 }
  };

  for (int i = 0; i < m_curves.size() - 1; i++) {
    m_curves[i].tessellate(params, geo);

    params.start_join = true;
    params.start_cap = false;
    params.is_first_segment = false;
  }

  params.end_cap = true;

  m_curves[m_curves.size() - 1].tessellate(params, geo);

  Renderer::draw(geo);

  m_curves.clear();

  for (int i = 0; i < vertices.size(); i++) {
    delete vertices[i];
  }
};
