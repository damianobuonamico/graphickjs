#include "freehand_entity.h"
#include "../../input/input_manager.h"
#include "../../../math/models/corner_detector.h"
#include "../../../math/models/path_fitter.h"
#include "../../../math/models/input_parser.h"
#include "../../settings.h"

FreehandEntity::FreehandEntity(const vec2& position, float pressure, double time)
  : m_transform(TransformComponent{ this, position }), m_creation_state({ vec2{ 0.0f }, 0.5f * pressure, time }), m_pressures({ 0.5f * pressure }) {
  WobbleSmoother::reset(
    { InputManager::pointer.type != InputManager::PointerType::Pen, 10.0f, 40.0f, 1.31f, 1.44f },
    m_position, pressure, time
  );
};

std::shared_ptr<ElementEntity> FreehandEntity::to_element() const {
  std::shared_ptr<ElementEntity> element = std::make_shared<ElementEntity>(m_transform.position().get());
  for (VertexEntity* vertex : m_vertices) {
    if (vertex->left() && vertex->right()) {
      element->add_vertex(std::make_shared<VertexEntity>(vertex->transform()->position().get(), vertex->transform()->left()->get(), vertex->transform()->right()->get(), vertex->taper().get()));
    } else if (vertex->left()) {
      element->add_vertex(std::make_shared<VertexEntity>(vertex->transform()->position().get(), vertex->transform()->left()->get(), true, vertex->taper().get()));
    } else if (vertex->right()) {
      element->add_vertex(std::make_shared<VertexEntity>(vertex->transform()->position().get(), vertex->transform()->right()->get(), false, vertex->taper().get()));
    } else {
      element->add_vertex(std::make_shared<VertexEntity>(vertex->transform()->position().get(), vertex->taper().get()));
    }
  }
  return element;
}

#if 0
void FreehandEntity::add_point(const vec2& position, float pressure, double time) {
  if (!m_creation_state.can_modify) {
    return;
  }

  pressure = 0.5f - 0.1f * std::cbrtf(10.0f * (0.5f - pressure));

  vec3 smoothed_point = WobbleSmoother::update(position, pressure, time);
  size_t points_size = m_creation_state.points.size();

  if (points_size > 1) {
    m_creation_state.points[points_size - 1].position = { smoothed_point.x, smoothed_point.y };
    m_creation_state.points[points_size - 1].pressure = smoothed_point.z;
  }

  m_creation_state.points.push_back({ position, pressure });

  for (int i = 0; i < m_curves.size(); i++) {
    delete m_curves[i];
  }
  m_curves.clear();

  for (int i = 0; i < m_vertices.size(); i++) {
    delete m_vertices[i];
  }
  m_vertices.clear();

  if (m_creation_state.points.size() < 3) {
    return;
  }

  for (int i = 1; i < m_creation_state.points.size() - 1; i++) {
    m_vertices.push_back(new VertexEntity(
      m_creation_state.points[i].position,
      -(m_creation_state.points[i + 1].position - m_creation_state.points[i - 1].position) * 0.33f,
      (m_creation_state.points[i + 1].position - m_creation_state.points[i].position) * 0.33f,
      m_creation_state.points[i].pressure
    ));
  }

  if (m_vertices.size() < 2) {
    return;
  }

  VertexEntity* last_vertex = nullptr;

  for (VertexEntity* vertex : m_vertices) {
    if (last_vertex) {
      m_curves.push_back(new BezierEntity{ *last_vertex, *vertex, this });
    }

    last_vertex = vertex;
  }
}
#else
void FreehandEntity::add_point(const vec2& position, float pressure, double time) {
  if (!m_creation_state.can_modify) {
    return;
  }

  console::log(pressure);
  pressure = 0.5f - 0.1f * std::cbrtf(10.0f * (0.5f - pressure));

  vec3 smoothed_point = WobbleSmoother::update(position, pressure, time);
  size_t points_size = m_creation_state.points.size();

  if (points_size > 1) {
    m_creation_state.points[points_size - 1].position = { smoothed_point.x, smoothed_point.y };
    m_creation_state.points[points_size - 1].pressure = smoothed_point.z;
    m_pressures[points_size - 1] = smoothed_point.z;
  }

  m_pressures.push_back(pressure);
  m_creation_state.points.push_back({ position, pressure });

  if (m_curves.size() > m_creation_state.last_committed_curve + 1) {
    for (int i = m_creation_state.last_committed_curve + 1; i < m_curves.size(); i++) {
      delete m_curves[i];
    }
    m_curves.erase(m_curves.begin() + 1 + m_creation_state.last_committed_curve, m_curves.end());
  }

  if (m_vertices.size() > m_creation_state.last_committed_vertex + 1) {
    for (int i = m_creation_state.last_committed_vertex + 1; i < m_vertices.size(); i++) {
      delete m_vertices[i];
    }
    m_vertices.erase(m_vertices.begin() + 1 + m_creation_state.last_committed_vertex, m_vertices.end());
    m_curves_indices.erase(m_curves_indices.begin() + 1 + m_creation_state.last_committed_vertex, m_curves_indices.end());
  }

  std::vector<uint> corners = detect_corners(
    m_creation_state.points,
    Settings::corners_radius_min,
    Settings::corners_radius_max,
    Settings::corners_angle_threshold,
    Settings::corners_min_distance,
    Settings::corners_samples_max
  );

  size_t corners_len = corners.size();

  if (corners_len < 2) {
    return;
  }

  size_t index_to_commit = corners_len > 3 ? corners_len - 3 : 0;
  uint point_to_commit = corners[index_to_commit];
  int curve_to_commit = m_creation_state.last_committed_curve;
  std::vector<PathBezier> curves;

  for (size_t i = 0; i < corners.size() - 1; i++) {
    uint start = corners[i];
    uint end = corners[i + 1];

    if (index_to_commit > 0 && i == index_to_commit) {
      curve_to_commit = curves.size() - 1;
    }

    fit_path(m_creation_state.points, start, end, Settings::max_fit_error, curves);
  }

  for (int i = 0; i < curves.size(); i++) {
    PathBezier& curve = curves[i];

    curve.start_index += m_creation_state.deleted_offset;
    curve.end_index += m_creation_state.deleted_offset;

    m_curves_indices.push_back(curve.start_index);

    if (m_vertices.empty()) {
      m_vertices.push_back(new VertexEntity(curve.p0, curve.p1 - curve.p0, false, 1.0f, this));
    } else {
      m_vertices[m_vertices.size() - 1]->set_right(curve.p1 - curve.p0);
      // m_vertices[m_vertices.size() - 1]->taper().move_to(0.5f * (m_vertices[m_vertices.size() - 1]->taper().get() + curve.pressure.x));
    }

    m_vertices.push_back(new VertexEntity(curve.p3, curve.p2 - curve.p3, true, curve.pressure.y, this));

    if (index_to_commit > 0 && i == curve_to_commit) {
      m_creation_state.last_committed_curve = m_curves.size() - 1;
      m_creation_state.last_committed_vertex = m_vertices.size() - 1;
    }
  }

  if (m_vertices.size() < 2) {
    return;
  }

  VertexEntity* last_vertex = nullptr;

  for (VertexEntity* vertex : m_vertices) {
    if (last_vertex) {
      m_curves.push_back(new BezierEntity{ *last_vertex, *vertex, this });
    }

    last_vertex = vertex;
  }

  console::log("curves_size", m_curves.size());
  console::log("indices_size", m_curves_indices.size());

  if (point_to_commit > 0) {
    m_creation_state.points.erase(m_creation_state.points.begin(), m_creation_state.points.begin() + point_to_commit);
    m_creation_state.deleted_offset += point_to_commit;
  }
}
#endif

void FreehandEntity::tessellate_outline(const vec4& color, float zoom, Geometry& geo) const {
  TessellationParams params = {
    m_transform.position().get(), zoom, MATH_PI / 20.0f,
    1.0f, color,
    JoinType::Bevel, CapType::Butt, 10.0f,
    false, false, false, false, true,
    { vec2{}, vec2{}, 0 }
  };

  for (auto* curve : m_curves) {
    curve->tessellate_outline(params, geo);
  }
}

void FreehandEntity::render(float zoom) const {
  std::vector<VertexEntity*> vertices{};

  if (m_curves.empty()) {
    return;
  }

  vec2 position = m_transform.position().get();

  TessellationParams params = {
    position, zoom, MATH_PI / 60.0f,
    5.0f, vec4(0.5f, 0.5f, 0.5f, 1.0f),
    JoinType::Round, CapType::Round, 10.0f,
    false, false, true, false, true,
    { vec2{}, vec2{}, 0 }
  };

  std::vector<float> triangulation_params{};
  std::vector<float> pressure_params{};

  for (int i = 0; i < m_curves.size(); i++) {
    BezierEntity* curve = m_curves[i];
    float a = curve->start().taper().get();
    float b = curve->end().taper().get();

    if (curve->strict_type() == BezierEntity::Type::Linear) {
    }

    std::vector<float> curve_params = curve->triangulation_params(params.zoom, params.facet_angle);
    triangulation_params.reserve(curve_params.size());
    pressure_params.reserve(curve_params.size());

    size_t start_index = m_curves_indices[i];
    size_t end_index = i < m_curves_indices.size() - 1 ? m_curves_indices[i + 1] : m_pressures.size() - 1;

    for (int j = 0; j < curve_params.size() - 1; j++) {
      triangulation_params.push_back(i + curve_params[j]);
      int range = end_index - start_index;
      float float_index = curve_params[j] * range;
      int index = (int)std::floor(start_index + float_index);
      pressure_params.push_back(lerp(m_pressures[index], m_pressures[index + 1], curve_params[j] / range));
    }
  }

  triangulation_params.push_back(m_curves.size());
  pressure_params.push_back(m_curves[m_curves.size() - 1]->end().taper().get());

  // std::vector<float> pressure_params(triangulation_params.size());
  // float max_t = triangulation_params[triangulation_params.size() - 1];
  // size_t pressures_size = m_pressures.size();

  // for (int i = 0; i < pressure_params.size() - 1; i++) {
  //   float t = triangulation_params[i] / max_t * pressures_size;
  //   int index = (int)std::floor(t);

  //   // console::log("t", );
  //   // console::log("index", index);
  //   pressure_params[i] = lerp(m_pressures[index], m_pressures[index], t - index);
  //   // console::log("pressure", m_pressures[index]);
  //   // console::log(pressure_params[i]);
  // }

  // pressure_params[pressure_params.size() - 1] = m_pressures[m_pressures.size() - 1];

  Geometry geo{};
  uint offset = 0;

  vec2 point, direction, normal;
  float width;

  {
    BezierEntity* curve = m_curves[0];

    width = params.width * pressure_params[0];
    point = params.offset + curve->get(0.0f);
    direction = curve->gradient(0.0f);
    normal = orthogonal(direction);
    normalize_length(normal, width, normal);

    geo.push_vertices({ { point - normal, params.color, -width }, { point + normal, params.color, width } });
    offset += 2;
  }
  const int kernel_size = 10;

  std::vector<float> pressure_values(pressure_params.size());

  for (int i = 0; i < pressure_params.size(); ++i) {
    int left_offset = i - kernel_size;
    int from = left_offset >= 0 ? left_offset : 0;
    int to = i + kernel_size + 1;

    int count = 0;
    float sum = 0;
    for (size_t j = from; j < to && j < pressure_params.size(); ++j) {
      sum += pressure_params[j];
      count += 1;
    }

    pressure_values[i] = sum / count;
  }

  for (int i = 1; i < triangulation_params.size(); i++) {
    float t = triangulation_params[i];
    int index = (int)std::ceil(t) - 1;
    t -= index;

    BezierEntity* curve = m_curves[index];

    width = params.width * pressure_values[i];
    point = params.offset + curve->get(t);
    direction = curve->gradient(t);
    normal = orthogonal(direction);
    normalize_length(normal, width, normal);

    geo.push_vertices({ { point - normal, params.color, -width }, { point + normal, params.color, width } });
    geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
    offset += 2;
  }

  // for (int i = 0; i < m_curves.size() - 1; i++) {
  //   m_curves[i]->tessellate(params, geo);

  //   params.start_join = true;
  //   params.start_cap = false;
  //   params.is_first_segment = false;
  // }

  // params.end_cap = true;

  // m_curves[m_curves.size() - 1]->tessellate(params, geo);

  Renderer::draw(geo);

  return;

  const vec4 outline_color{ 0.22f, 0.76f, 0.95f, 1.0f };
  Geometry outline_geometry{ GL_LINES };
  tessellate_outline(outline_color, zoom, outline_geometry);

  InstancedGeometry vertex_geometry{};
  InstancedGeometry handle_geometry{};
  vertex_geometry.push_quad(vec2{ 0.0f }, 3.5f / zoom, outline_color);
  handle_geometry.push_circle(vec2{ 0.0f }, 2.5f / zoom, outline_color, 10);

  for (auto* vertex : m_vertices) {
    vec2 vertex_position = position + vertex->transform()->position().get();
    vertex_geometry.push_instance(vertex_position);

    HandleEntity* left = vertex->left();
    HandleEntity* right = vertex->right();

    uint32_t vertex_index = outline_geometry.offset();

    if (left || right) {
      outline_geometry.push_vertex({ vertex_position, outline_color });
    }

    if (left) {
      vec2 handle_position = vertex_position + left->transform()->position().get();
      handle_geometry.push_instance(handle_position);

      outline_geometry.push_indices({ vertex_index, outline_geometry.offset() });
      outline_geometry.push_vertex({ handle_position, outline_color });
    }
    if (right) {
      vec2 handle_position = vertex_position + right->transform()->position().get();
      handle_geometry.push_instance(handle_position);

      outline_geometry.push_indices({ vertex_index, outline_geometry.offset() });
      outline_geometry.push_vertex({ handle_position, outline_color });
    }
  }

  Renderer::draw(outline_geometry);
  Renderer::draw(vertex_geometry);
  Renderer::draw(handle_geometry);
};
