#include "freehand_entity.h"
#include "../../input/input_manager.h"

FreehandEntity::FreehandEntity(const vec2& position, float pressure, double time)
  : m_transform(TransformComponent{ this }), m_position(position), m_points({ {m_position, pressure} }) {
  console::log("FreehandEntity created");
  WobbleSmoother::reset({ InputManager::pointer.type != InputManager::PointerType::Pen, 10.0f, 40.0f, 1.31f, 1.44f }, m_position, pressure, time);
};

void FreehandEntity::add_point(const vec2& position, float pressure, double time) {
  vec3 smoothed_point = WobbleSmoother::update(m_position + position, pressure, time);

  if (m_points.size() > 1) {
    m_points[m_points.size() - 1].position = { smoothed_point.x, smoothed_point.y };
    m_points[m_points.size() - 1].pressure = smoothed_point.z;
  }

  m_points.push_back({ m_position + position, pressure });
}

void FreehandEntity::render(float zoom) const {
  // std::vector<FreehandPathPoint> points = simplify_first ? simplify_path(smooth_freehand_path(m_points, 4), simplification_tolerance, true) : smooth_freehand_path(m_points, 4);
  std::vector<FreehandPathPoint> points = smooth_freehand_path(m_points, 10);
  // std::vector<FreehandPathPoint> points = simplify_path(smooth_freehand_path(m_points, 10), simplification_tolerance, true);

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
          vertices.push_back(new VertexEntity{ curve.p0, curve.p1 - curve.p0, false, curve.p0_pressure });
        } else {
          vertices[vertices.size() - 1]->set_right(curve.p1 - curve.p0);
          vertices[vertices.size() - 1]->taper().move_to(0.5f * (vertices[vertices.size() - 1]->taper().get() + curve.p0_pressure));
        }

        vertices.push_back(new VertexEntity{ curve.p3, curve.p2 - curve.p3, true, curve.p3_pressure });
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


  m_curves.clear();

  vec4 vertex_color{ 239.0f / 255.0f, 89.0f / 255.0f, 88.0f / 255.0f, 1.0f };


  for (int i = 0; i < vertices.size(); i++) {
    geo.push_circle(vertices[i]->transform().position().get(), 1.0f, vertex_color, 20);
    delete vertices[i];
  }

  for (uint i : corners) {
    geo.push_circle(points[i].position, 1.0f, vec4(92.0f / 255.0f, 200.0f / 255.0f, 134.0f / 255.0f, 1.0f), 20);
  }

  for (auto& point : points) {
    geo.push_circle(point.position, 0.5f, vec4(40.0f / 255.0f, 87.0f / 255.0f, 147.0f / 255.0f, 1.0f), 10);
  }

  Renderer::draw(geo);
};
