#include "debugger.h"

#ifdef GK_CONF_DEBUG

#include "../renderer/renderer.h"
#include "../editor/scene/entities/bezier_entity.h"
#include "../editor/editor.h"
#include "../editor/input/input_manager.h"

Debugger* Debugger::s_instance = nullptr;

void Debugger::init() {
  assert(!s_instance);
  s_instance = new Debugger();
}

void Debugger::shutdown() {
  delete s_instance;
}

void Debugger::update() {
  get()->render();
}

void Debugger::render() {
  m_frame_offset_left = 0.0f;
  m_frame_offset_right = 0.0f;

  float frame_size = 200.0f;
  const BezierEntity* hovered = dynamic_cast<BezierEntity*>(InputManager::hover.entity());
  if (hovered && hovered->type() != BezierEntity::Type::Cubic) {
    hovered = nullptr;
  }

  if (!hovered && !Editor::scene.selection.empty()) {
    ElementEntity* element = dynamic_cast<ElementEntity*>(Editor::scene.selection.begin()->second);
    if (element && !element->selection()->empty()) {
      VertexEntity* vertex = dynamic_cast<VertexEntity*>(element->selection()->entities()[0]);
      for (auto it = element->curves_begin(); it != element->curves_end(); ++it) {
        if (it->start().id == vertex->id) {
          const BezierEntity& bezier = *it;
          if (bezier.type() == BezierEntity::Type::Cubic) {
            hovered = &bezier;
            break;
          }
        } else if (it->end().id == vertex->id) {
          const BezierEntity& bezier = *it;
          if (bezier.type() == BezierEntity::Type::Cubic) {
            hovered = &bezier;
            break;
          }
        }
      }
    }
  }

  push_frame(vec2{ frame_size * 2 });
  if (hovered) {
    render_bezier_hodograph(hovered);
  }

  push_frame(vec2{ frame_size });
  if (hovered) {
    render_bezier_curvature(hovered);
  }

  push_frame(vec2{ frame_size * 2 }, false);
  if (hovered) {
    render_bezier_geometry(hovered);
  }

  push_frame(vec2{ frame_size }, false);
  if (hovered) {
    render_bezier_triangulation(hovered);
  }

  push_frame(vec2{ frame_size }, false);
  if (hovered) {
    render_bezier_outline(hovered);
  }

  return;
}

void Debugger::push_frame(const vec2& size, bool align_right) {
  vec2 viewport_size = Editor::viewport.size();
  m_frame_width = size.x - 2.0f * m_padding;
  m_cursor = vec2{ m_padding, m_padding };

  if (align_right) {
    Renderer::push_overlay_layer(vec2{ viewport_size.x - size.x, m_frame_offset_right });
    m_frame_offset_right += size.y + m_padding / 2.0f;
  } else {
    Renderer::push_overlay_layer(vec2{ 0.0f, m_frame_offset_left });
    m_frame_offset_left += size.y + m_padding / 2.0f;
  }

  Geometry frame_geometry{};
  frame_geometry.push_quad(Box{ vec2{0.0f}, size }, vec4{ 0.0f, 0.0f, 0.0f, 0.5f });

  Renderer::draw(frame_geometry);
}

void Debugger::render_bezier_outline(const BezierEntity* entity) {
  Box box = entity->bounding_box();
  vec2 box_size = box.max - box.min;
  vec2 sizes_ratio = box_size / m_frame_width;

  float ratio = 1;
  vec2 center{ 0.0f };

  if (sizes_ratio.x > sizes_ratio.y) {
    ratio = sizes_ratio.x;
    center = m_cursor + vec2{ 0.0f, (m_frame_width - box_size.y / ratio) / 2.0f };
  } else {
    ratio = sizes_ratio.y;
    center = m_cursor + vec2{ (m_frame_width - box_size.x / ratio) / 2.0f, 0.0f };
  }

  vec2 A = center + (entity->p0() - box.min) / ratio;
  vec2 B = center + (entity->p1() - box.min) / ratio;
  vec2 C = center + (entity->p2() - box.min) / ratio;
  vec2 D = center + (entity->p3() - box.min) / ratio;

  int resolution = 50;
  vec4 color{ 1.0f, 1.0f, 1.0f, 0.7f };
  Geometry geo{ GL_LINES };
  std::vector<Vertex> vertices(resolution + 1);

  for (int i = 0; i <= resolution; i++) {
    float t = (float)i / (float)resolution;
    vertices[i].position = bezier(A, B, C, D, t);
    vertices[i].color = color;
    vertices[i].normal = 0.0f;
  }

  InstancedGeometry points_geo{};
  points_geo.push_circle(vec2{ 0.0f }, 2.5f, color, 10);
  std::vector<vec2> debug_points{};

  debug_points.reserve(m_t_values.size());
  for (float t : m_t_values) {
    vec2 point = bezier(A, B, C, D, t);
    debug_points.push_back(point);
  }

  geo.push_line_strip(vertices);
  points_geo.push_instances(debug_points);

  Renderer::draw(geo);
  Renderer::draw(points_geo);
}

static vec2 t_from_theta(const vec2& v1, const vec2& v2, const vec2& v3, const vec2& v4, float theta) {
  vec2 A = 3.0f * (-v1 + 3.0f * v2 - 3.0f * v3 + v4);
  vec2 B = 6.0f * (v1 - 2.0f * v2 + v3);
  vec2 C = -3.0f * (v1 - v2);

  float tan = tanf(theta);

  float a = A.y - tan * A.x;
  float b = B.y - tan * B.x;
  float c = C.y - tan * C.x;

  if (is_almost_zero(a)) {
    if (is_almost_zero(b)) {
      return { -1.0f, -1.0f };
    }
    return { -c / b, -1.0f };
  }

  float delta = b * b - 4.0f * a * c;

  if (is_almost_zero(delta)) {
    return { -b / (2.0f * a), -1.0f };
  } else if (delta > 0.0f) {
    float sqrt_delta = sqrtf(delta);
    float t1 = (-b + sqrt_delta) / (2.0f * a);
    float t2 = (-b - sqrt_delta) / (2.0f * a);

    return { t1, t2 };
  }

  return { -1.0f, -1.0f };
}


void Debugger::render_bezier_hodograph(const BezierEntity* entity) {
  int resolution = 50;
  std::vector<vec2> points(resolution + 1);
  Box box = { vec2{ 0.0f }, vec2{ 0.0f } };

  vec2 A = entity->p0();
  vec2 B = entity->p1();
  vec2 C = entity->p2();
  vec2 D = entity->p3();

  for (int i = 0; i <= resolution; i++) {
    float t = (float)i / (float)resolution;
    vec2 point = bezier_derivative(A, B, C, D, t);

    min(box.min, point, box.min);
    max(box.max, point, box.max);
    points[i] = point;
  }

  vec2 box_size = box.max - box.min;
  vec2 sizes_ratio = box_size / m_frame_width;

  float ratio = 1;
  vec2 center{ 0.0f };

  if (sizes_ratio.x > sizes_ratio.y) {
    ratio = sizes_ratio.x;
    center = m_cursor + vec2{ 0.0f, (m_frame_width - box_size.y / ratio) / 2.0f };
  } else {
    ratio = sizes_ratio.y;
    center = m_cursor + vec2{ (m_frame_width - box_size.x / ratio) / 2.0f, 0.0f };
  }

  vec4 color{ 1.0f, 1.0f, 1.0f, 0.7f };
  Geometry geo{ GL_LINES };
  std::vector<Vertex> vertices(resolution + 1);

  for (int i = 0; i < points.size(); i++) {
    vertices[i].position = center + (points[i] - box.min) / ratio;
    vertices[i].color = color;
  }

  InstancedGeometry points_geo{};
  points_geo.push_circle(vec2{ 0.0f }, 2.5f, color, 10);
  std::vector<vec2> debug_points{};

  vec2 origin = center + (vec2{ 0.0f } - box.min) / ratio;

  debug_points.push_back(origin);

  int increments = 30;
  float angle_increment = MATH_TWO_PI / (float)increments;
  vec4 line_color{ 0.3f, 0.3f, 0.3f, 0.5f };
  Box boundary{ vec2{ m_padding }, vec2{ m_padding + m_frame_width } };

  for (int i = 0; i < increments; i++) {
    float angle = (float)i * angle_increment;
    draw_polar_line(origin, angle, boundary, line_color, geo);
  }

  vec4 green{ 0.0f, 1.0f, 0.5f, 0.5f };
  vec4 red{ 1.0f, 0.0f, 0.5f, 0.5f };
  vec4 purple{ 5.0f, 0.0f, 1.0f, 0.5f };

  std::vector<float> inflections = entity->inflections();
  std::vector<vec2> inflection_points(inflections.size());
  std::vector<float> turning_points(inflections.size());

  for (int i = 0; i < inflections.size(); i++) {
    inflection_points[i] = bezier(A, B, C, D, inflections[i]);
  }

  bool is_derivative_clockwise = clockwise(inflection_points);

  for (int i = 0; i < inflections.size(); i++) {
    vec2 point = bezier_derivative(A, B, C, D, inflections[i]);
    debug_points.push_back(center + (point - box.min) / ratio);
    float angle;

    if (is_almost_zero(point)) {
      vec2 curvature = bezier_second_derivative(A, B, C, D, inflections[i]);
      angle = std::atan2f(curvature.y, curvature.x);
    } else {
      angle = std::atan2f(point.y, point.x);
    }

    draw_polar_line(origin, angle, boundary, green, geo);
    turning_points[i] = angle;
  }

  const float max_angle_difference = std::max(max_angle, MATH_PI / 300.0f);
  m_t_values.clear();

  for (int i = 0; i < (int)turning_points.size() - 1; i++) {
    float difference = turning_points[i + 1] - turning_points[i];

    vec2 midpoint = bezier_derivative(A, B, C, D, (inflections[i] + inflections[i + 1]) / 2);
    float mid_angle = std::atan2f(midpoint.y, midpoint.x);

    float k1 = (mid_angle - turning_points[i]) / difference;
    float k2 = (mid_angle + MATH_TWO_PI - turning_points[i]) / difference;

    if (!(is_normalized(k1) || is_normalized(k2))) {
      if (difference < 0.0f) {
        difference += MATH_TWO_PI;
      } else {
        difference -= MATH_TWO_PI;
      }
    }

    int increments = std::max(abs((int)ceilf(difference / max_angle_difference)), 1);
    float increment = difference / (float)increments;

    m_t_values.reserve(increments);

    m_t_values.push_back(inflections[i]);
    for (int j = 1; j < increments; j++) {
      float theta = turning_points[i] + (float)j * increment;

      if (theta > MATH_TWO_PI) {
        theta -= MATH_TWO_PI;
      } else if (theta < 0.0f) {
        theta += MATH_TWO_PI;
      }

      draw_polar_line(origin, theta, boundary, i % 2 == 0 ? red : green, geo);
      vec2 t_values = t_from_theta(A, B, C, D, theta);

      float last_t = m_t_values[m_t_values.size() - 1];
      bool is_t1_bad = !is_in_range(t_values.x, last_t, inflections[i + 1], false);
      bool is_t2_bad = !is_in_range(t_values.y, last_t, inflections[i + 1], false);

      if (is_t1_bad) {
        if (!is_t2_bad) {
          m_t_values.push_back(t_values.y);
        }
        continue;
      } else if (is_t2_bad) {
        if (!is_t1_bad) {
          m_t_values.push_back(t_values.x);
        }
        continue;
      }


      if (t_values.x - last_t < t_values.y - last_t) {
        m_t_values.push_back(t_values.x);
      } else {
        m_t_values.push_back(t_values.y);
      }
    }
  }

  m_t_values.push_back(inflections[inflections.size() - 1]);

  debug_points.reserve(m_t_values.size());
  for (float t : m_t_values) {
    vec2 point = bezier_derivative(A, B, C, D, t);
    debug_points.push_back(center + (point - box.min) / ratio);
  }

  geo.push_line_strip(vertices);
  points_geo.push_instances(debug_points);

  Renderer::draw(geo);
  Renderer::draw(points_geo);
}

void Debugger::render_bezier_curvature(const BezierEntity* entity) {
  int resolution = 50;
  std::vector<vec2> points(resolution + 1);
  Box box = { vec2{ 0.0f }, vec2{ 0.0f } };

  vec2 A = entity->p0();
  vec2 B = entity->p1();
  vec2 C = entity->p2();
  vec2 D = entity->p3();

  for (int i = 0; i <= resolution; i++) {
    float t = (float)i / (float)resolution;
    vec2 point = bezier_second_derivative(A, B, C, D, t);

    min(box.min, point, box.min);
    max(box.max, point, box.max);
    points[i] = point;
  }

  vec2 box_size = box.max - box.min;
  vec2 sizes_ratio = box_size / m_frame_width;

  float ratio = 1;
  vec2 center{ 0.0f };

  if (sizes_ratio.x > sizes_ratio.y) {
    ratio = sizes_ratio.x;
    center = m_cursor + vec2{ 0.0f, (m_frame_width - box_size.y / ratio) / 2.0f };
  } else {
    ratio = sizes_ratio.y;
    center = m_cursor + vec2{ (m_frame_width - box_size.x / ratio) / 2.0f, 0.0f };
  }

  vec4 color{ 1.0f, 1.0f, 1.0f, 0.7f };
  Geometry geo{ GL_LINES };
  std::vector<Vertex> vertices(resolution + 1);

  for (int i = 0; i < points.size(); i++) {
    vertices[i].position = center + (points[i] - box.min) / ratio;
    vertices[i].color = color;
  }

  InstancedGeometry points_geo{};
  points_geo.push_circle(vec2{ 0.0f }, 2.5f, color, 10);
  std::vector<vec2> debug_points{};

  vec2 origin = center + (vec2{ 0.0f } - box.min) / ratio;

  debug_points.push_back(origin);

  int increments = 30;
  float angle_increment = MATH_TWO_PI / (float)increments;
  vec4 line_color{ 0.3f, 0.3f, 0.3f, 0.5f };
  Box boundary{ vec2{ m_padding }, vec2{ m_padding + m_frame_width } };

  for (int i = 0; i < increments; i++) {
    float angle = (float)i * angle_increment;
    draw_polar_line(origin, angle, boundary, line_color, geo);
  }

  vec4 green{ 0.0f, 1.0f, 0.5f, 0.5f };
  vec4 red{ 1.0f, 0.0f, 0.5f, 0.5f };
  vec4 purple{ 5.0f, 0.0f, 1.0f, 0.5f };

  geo.push_line_strip(vertices);
  points_geo.push_instances(debug_points);

  Renderer::draw(geo);
  Renderer::draw(points_geo);
}

void Debugger::render_bezier_geometry(const BezierEntity* entity) {
  Box box = entity->bounding_box();
  vec2 box_size = box.max - box.min;
  vec2 sizes_ratio = box_size / (m_frame_width - stroke_width * 2);

  float ratio = 1;
  vec2 center{ 0.0f };

  if (sizes_ratio.x > sizes_ratio.y) {
    ratio = sizes_ratio.x;
    center = m_cursor + stroke_width + vec2{ 0.0f, (m_frame_width - stroke_width * 2 - box_size.y / ratio) / 2.0f };
  } else {
    ratio = sizes_ratio.y;
    center = m_cursor + stroke_width + vec2{ (m_frame_width - stroke_width * 2 - box_size.x / ratio) / 2.0f, 0.0f };
  }

  vec2 A = center + (entity->p0() - box.min) / ratio;
  vec2 B = center + (entity->p1() - box.min) / ratio;
  vec2 C = center + (entity->p2() - box.min) / ratio;
  vec2 D = center + (entity->p3() - box.min) / ratio;

  vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
  Geometry geo{};

  uint32_t offset = 0;

  {
    vec2 point = bezier(A, B, C, D, 0.0f);
    vec2 tangent = bezier_derivative(A, B, C, D, 0.0f);
    if (is_almost_zero(tangent)) {
      tangent = bezier_second_derivative(A, B, C, D, 0.0f);
    }
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({
      {point + normal, color, stroke_width},
      {point - normal, color, -stroke_width}
      });

    offset += 2;
  }

  for (size_t i = 1; i < m_t_values.size() - 1; i++) {
    vec2 point = bezier(A, B, C, D, m_t_values[i]);
    vec2 tangent = bezier_derivative(A, B, C, D, m_t_values[i]);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({
      {point + normal, color, stroke_width},
      {point - normal, color, -stroke_width}
      });

    geo.push_indices({
      offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1
      });

    offset += 2;
  }

  {
    vec2 point = bezier(A, B, C, D, 1.0f);
    vec2 tangent = bezier_derivative(A, B, C, D, 1.0f);
    if (is_almost_zero(tangent)) {
      tangent = bezier_second_derivative(A, B, C, D, 1.0f);
    }
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({
      {point + normal, color, stroke_width},
      {point - normal, color, -stroke_width}
      });

    geo.push_indices({
      offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1
      });

    offset += 2;
  }

  Renderer::draw(geo);
}

void Debugger::render_bezier_triangulation(const BezierEntity* entity) {
  Box box = entity->bounding_box();
  vec2 box_size = box.max - box.min;
  vec2 sizes_ratio = box_size / (m_frame_width - stroke_width * 2);

  float ratio = 1;
  vec2 center{ 0.0f };

  if (sizes_ratio.x > sizes_ratio.y) {
    ratio = sizes_ratio.x;
    center = m_cursor + stroke_width + vec2{ 0.0f, (m_frame_width - stroke_width * 2 - box_size.y / ratio) / 2.0f };
  } else {
    ratio = sizes_ratio.y;
    center = m_cursor + stroke_width + vec2{ (m_frame_width - stroke_width * 2 - box_size.x / ratio) / 2.0f, 0.0f };
  }

  vec2 A = center + (entity->p0() - box.min) / ratio;
  vec2 B = center + (entity->p1() - box.min) / ratio;
  vec2 C = center + (entity->p2() - box.min) / ratio;
  vec2 D = center + (entity->p3() - box.min) / ratio;

  vec4 color{ 1.0f, 1.0f, 1.0f, 0.7f };
  Geometry geo{ GL_TRIANGLES };

  uint32_t offset = 0;

  {
    vec2 point = bezier(A, B, C, D, 0.0f);
    vec2 tangent = bezier_derivative(A, B, C, D, 0.0f);
    if (is_almost_zero(tangent)) {
      tangent = bezier_second_derivative(A, B, C, D, 0.0f);
    }
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({
      {point + normal, color},
      {point - normal, color}
      });

    offset += 2;
  }

  for (size_t i = 1; i < m_t_values.size() - 1; i++) {
    vec2 point = bezier(A, B, C, D, m_t_values[i]);
    vec2 tangent = bezier_derivative(A, B, C, D, m_t_values[i]);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({
      {point + normal, color},
      {point - normal, color}
      });

    geo.push_indices({
      offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1
      });

    offset += 2;
  }

  {
    vec2 point = bezier(A, B, C, D, 1.0f);
    vec2 tangent = bezier_derivative(A, B, C, D, 1.0f);
    if (is_almost_zero(tangent)) {
      tangent = bezier_second_derivative(A, B, C, D, 1.0f);
    }
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({
      {point + normal, color},
      {point - normal, color}
      });

    geo.push_indices({
      offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1
      });

    offset += 2;
  }

  Renderer::draw(geo.wireframe());
}

void Debugger::draw_polar_line(const vec2& origin, float angle, const Box& boundary, const vec4& color, Geometry& geo) {
  float tan = std::tanf(angle);

  vec2 p0 = origin;
  vec2 p1;

  if (angle <= -MATH_PI / 2.0f) {
    p1 = { boundary.min.x, tan * (boundary.min.x - origin.x) + origin.y };
  } else if (angle <= MATH_PI / 2.0f || angle >= MATH_PI * 3.0f / 2.0f) {
    p1 = { boundary.max.x, tan * (boundary.max.x - origin.x) + origin.y };
  } else {
    p1 = { boundary.min.x, tan * (boundary.min.x - origin.x) + origin.y };
  }

  if (p0.y >= boundary.max.y && p1.y > boundary.max.y) {
    return;
  }
  if (p0.y <= boundary.min.y && p1.y < boundary.min.y) {
    return;
  }
  if (p0.x >= boundary.max.x && p1.x > boundary.max.x) {
    return;
  }
  if (p0.x <= boundary.min.x && p1.x < boundary.min.x) {
    return;
  }

  if (p1.y < boundary.min.y) {
    std::vector<vec2> intersections = line_line_intersection_points(Box{ p0, p1 }, Box{ boundary.min, vec2{ boundary.max.x, boundary.min.x } });
    if (!intersections.empty()) {
      p1 = intersections[0];
    }
  } else if (p1.y > boundary.max.y) {
    std::vector<vec2> intersections = line_line_intersection_points(Box{ p0, p1 }, Box{ vec2{ boundary.min.x, boundary.max.y }, boundary.max });
    if (!intersections.empty()) {
      p1 = intersections[0];
    }
  }

  geo.push_line(p0, p1, color);
}

#endif
