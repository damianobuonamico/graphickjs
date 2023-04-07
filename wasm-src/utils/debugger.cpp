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
  zero(m_frame_offset_left);
  zero(m_frame_offset_right);

  float frame_size = 200.0f;
  const BezierEntity* hovered = dynamic_cast<BezierEntity*>(InputManager::hover.entity());
  if (hovered && hovered->type() != BezierEntity::Type::Cubic) {
    hovered = nullptr;
  }

  if (!hovered && !Editor::scene().selection.empty()) {
    ElementEntity* element = dynamic_cast<ElementEntity*>(Editor::scene().selection.begin()->second);
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

  push_frame(vec2{ frame_size });
  if (hovered) {
    render_bezier_hodograph(hovered);
  }

  push_frame(vec2{ frame_size });
  if (hovered) {
    render_bezier_curvature(hovered);
  }

  push_frame(vec2{ frame_size }, false);
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
  vec2 viewport_size = Editor::scene().viewport.size();
  m_frame_width = size.x - 2.0f * m_padding;
  m_cursor = vec2{ m_padding, m_padding };

  if (align_right) {
    if (m_frame_offset_right.y + size.y > viewport_size.y) {
      m_frame_offset_right.y = 0.0f;
      m_frame_offset_right.x -= size.x + m_padding / 2.0f;
    }

    Renderer::push_overlay_layer(vec2{ viewport_size.x - size.x + m_frame_offset_right.x, m_frame_offset_right.y });
    m_frame_offset_right.y += size.y + m_padding / 2.0f;
  } else {
    if (m_frame_offset_left.y + size.y > viewport_size.y) {
      m_frame_offset_left.y = 0.0f;
      m_frame_offset_left.x += size.x + m_padding / 2.0f;
    }

    Renderer::push_overlay_layer(vec2{ m_frame_offset_left.x, m_frame_offset_left.y });
    m_frame_offset_left.y += size.y + m_padding / 2.0f;
  }

  Geometry frame_geometry{};
  frame_geometry.push_quad(Box{ vec2{0.0f}, size }, vec4{ 0.0f, 0.0f, 0.0f, 0.5f });

  Renderer::draw(frame_geometry);
}

void Debugger::render_bezier_outline(const BezierEntity* entity) {
  const int resolution = 50;

  vec4 color{ 1.0f, 1.0f, 1.0f, 0.7f };
  Geometry geo{ GL_LINES };
  InstancedGeometry points_geo{};
  points_geo.push_circle(vec2{ 0.0f }, 2.5f, color, 10);

  Box box = entity->bounding_box();

  float ratio;
  vec2 center;
  Box boundaries;

  calculate_frame_boundaries(box, 0.0f, center, &ratio, &boundaries);

  vec2 A = center + (entity->p0() - box.min) / ratio;
  vec2 B = center + (entity->p1() - box.min) / ratio;
  vec2 C = center + (entity->p2() - box.min) / ratio;
  vec2 D = center + (entity->p3() - box.min) / ratio;

  std::vector<Vertex> vertices(resolution + 1);

  for (int i = 0; i <= resolution; i++) {
    float t = (float)i / (float)resolution;
    vertices[i].position = bezier(A, B, C, D, t);
    vertices[i].color = color;
  }

  points_geo.reserve_instances(m_t_values.size());
  for (float t : m_t_values) {
    vec2 point = bezier(A, B, C, D, t);
    points_geo.push_instance(point);
  }

  geo.push_line_strip(vertices);

  Renderer::draw(geo);
  Renderer::draw(points_geo);
}

void Debugger::render_bezier_hodograph(const BezierEntity* entity) {
  const int resolution = 50;

  vec4 white{ 1.0f, 1.0f, 1.0f, 0.7f };
  vec4 green{ 0.0f, 1.0f, 0.5f, 0.5f };
  vec4 red{ 1.0f, 0.0f, 0.5f, 0.5f };

  Geometry geo{ GL_LINES };
  InstancedGeometry points_geo{};
  points_geo.push_circle(vec2{ 0.0f }, 2.5f, white, 10);

  std::vector<vec2> points(resolution + 1);
  Box box = { vec2{ 0.0f }, vec2{ 0.0f } };
  vec2 A = entity->p0();
  vec2 B = entity->p1();
  vec2 C = entity->p2();
  vec2 D = entity->p3();

  for (int i = 0; i <= resolution; i++) {
    vec2 point = bezier_derivative(A, B, C, D, (float)i / (float)resolution);
    min(box.min, point, box.min);
    max(box.max, point, box.max);
    points[i] = point;
  }

  float ratio;
  vec2 center;
  Box boundaries;
  vec2 box_size = box.max - box.min;

  calculate_frame_boundaries(box, 0.0f, center, &ratio, &boundaries);

  std::vector<Vertex> vertices(resolution + 1);
  for (int i = 0; i < points.size(); i++) {
    vertices[i].position = center + (points[i] - box.min) / ratio;
    vertices[i].color = white;
  }

  vec2 origin = center - box.min / ratio;
  points_geo.push_instance(origin);
  draw_polar_plane(origin, boundaries, geo);

  std::vector<vec2> turning_angles = entity->turning_angles();
  for (const vec2& angle : turning_angles) {
    draw_polar_line(origin, angle.y, boundaries, green, geo);
  }

  std::vector<float> t_values = m_t_values = entity->triangulation_params({ 1.0f, max_angle });
  for (int i = 0; i < (int)turning_angles.size() - 1; i++) {
    float difference = turning_angles[i + 1].y - turning_angles[i].y;

    vec2 midpoint = bezier_derivative(A, B, C, D, (turning_angles[i].x + turning_angles[i + 1].x) / 2);
    float mid_angle = std::atan2f(midpoint.y, midpoint.x);

    float k1 = (mid_angle - turning_angles[i].y) / difference;
    float k2 = (mid_angle + MATH_TWO_PI - turning_angles[i].y) / difference;

    if (!(is_normalized(k1) || is_normalized(k2))) {
      if (difference < 0.0f) {
        difference += MATH_TWO_PI;
      } else {
        difference -= MATH_TWO_PI;
      }
    }

    int increments = std::max(std::abs((int)std::ceilf(difference / std::max(max_angle, GEOMETRY_MIN_FACET_ANGLE))), 1);
    float increment = difference / (float)increments;

    for (int j = 1; j < increments; j++) {
      float theta = turning_angles[i].y + (float)j * increment;

      if (theta > MATH_TWO_PI) {
        theta -= MATH_TWO_PI;
      } else if (theta < 0.0f) {
        theta += MATH_TWO_PI;
      }

      draw_polar_line(origin, theta, boundaries, i % 2 == 0 ? red : green, geo);
    }
  }

  geo.push_line_strip(vertices);

  points_geo.reserve_instances(t_values.size());
  for (float t : t_values) {
    vec2 point = bezier_derivative(A, B, C, D, t);
    points_geo.push_instance(center + (point - box.min) / ratio);
  }

  Renderer::draw(geo);
  Renderer::draw(points_geo);
}

void Debugger::render_bezier_curvature(const BezierEntity* entity) {
  const int resolution = 50;

  vec4 color{ 1.0f, 1.0f, 1.0f, 0.7f };
  Geometry geo{ GL_LINES };
  InstancedGeometry points_geo{};
  points_geo.push_circle(vec2{ 0.0f }, 2.5f, color, 10);

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

  float ratio;
  vec2 center;
  Box boundaries;

  calculate_frame_boundaries(box, 0.0f, center, &ratio, &boundaries);

  std::vector<Vertex> vertices(resolution + 1);
  for (int i = 0; i < points.size(); i++) {
    vertices[i].position = center + (points[i] - box.min) / ratio;
    vertices[i].color = color;
  }

  vec2 origin = center + (vec2{ 0.0f } - box.min) / ratio;
  points_geo.push_instance(origin);
  draw_polar_plane(origin, boundaries, geo);

  geo.push_line_strip(vertices);

  Renderer::draw(geo);
  Renderer::draw(points_geo);
}

void Debugger::render_bezier_geometry(const BezierEntity* entity) {
  vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
  Geometry geo{};

  Box box = entity->bounding_box();

  float ratio;
  vec2 center;
  Box boundaries;

  calculate_frame_boundaries(box, stroke_width, center, &ratio, &boundaries);

  vec2 A = center + (entity->p0() - box.min) / ratio;
  vec2 B = center + (entity->p1() - box.min) / ratio;
  vec2 C = center + (entity->p2() - box.min) / ratio;
  vec2 D = center + (entity->p3() - box.min) / ratio;

  uint32_t offset = 0;

  {
    vec2 point = bezier(A, B, C, D, 0.0f);
    vec2 tangent = bezier_derivative(A, B, C, D, 0.0f);
    if (is_almost_zero(tangent)) {
      tangent = bezier_second_derivative(A, B, C, D, 0.0f);
    }
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({ {point + normal, color, stroke_width},{point - normal, color, -stroke_width} });
    offset += 2;
  }

  for (size_t i = 1; i < m_t_values.size() - 1; i++) {
    vec2 point = bezier(A, B, C, D, m_t_values[i]);
    vec2 tangent = bezier_derivative(A, B, C, D, m_t_values[i]);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({ {point + normal, color, stroke_width},{point - normal, color, -stroke_width} });
    geo.push_indices({ offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1 });
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

    geo.push_vertices({ {point + normal, color, stroke_width},{point - normal, color, -stroke_width} });
    geo.push_indices({ offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1 });
    offset += 2;
  }

  Renderer::draw(geo);
}

void Debugger::render_bezier_triangulation(const BezierEntity* entity) {
  vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
  Geometry geo{};

  Box box = entity->bounding_box();

  float ratio;
  vec2 center;
  Box boundaries;

  calculate_frame_boundaries(box, stroke_width, center, &ratio, &boundaries);

  vec2 A = center + (entity->p0() - box.min) / ratio;
  vec2 B = center + (entity->p1() - box.min) / ratio;
  vec2 C = center + (entity->p2() - box.min) / ratio;
  vec2 D = center + (entity->p3() - box.min) / ratio;

  uint32_t offset = 0;

  {
    vec2 point = bezier(A, B, C, D, 0.0f);
    vec2 tangent = bezier_derivative(A, B, C, D, 0.0f);
    if (is_almost_zero(tangent)) {
      tangent = bezier_second_derivative(A, B, C, D, 0.0f);
    }
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({ {point + normal, color}, {point - normal, color} });
    offset += 2;
  }

  for (size_t i = 1; i < m_t_values.size() - 1; i++) {
    vec2 point = bezier(A, B, C, D, m_t_values[i]);
    vec2 tangent = bezier_derivative(A, B, C, D, m_t_values[i]);
    normalize(tangent, tangent);
    vec2 normal = stroke_width * orthogonal(tangent);

    geo.push_vertices({ {point + normal, color}, {point - normal, color} });
    geo.push_indices({ offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1 });
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

    geo.push_vertices({ {point + normal, color},{point - normal, color} });
    geo.push_indices({ offset - 2, offset - 1, offset + 0, offset + 0, offset + 1, offset - 1 });
    offset += 2;
  }

  Renderer::draw(geo.wireframe());
}

void Debugger::calculate_frame_boundaries(const Box& box, float padding, vec2& center, float* ratio, Box* boundaries) {
  float frame_width = m_frame_width - padding * 2.0f;

  vec2 box_size = box.max - box.min;
  vec2 sizes_ratio = box_size / frame_width;

  if (sizes_ratio.x > sizes_ratio.y) {
    *ratio = sizes_ratio.x;
    center = m_cursor + padding + vec2{ 0.0f, (frame_width - box_size.y / *ratio) / 2.0f };
  } else {
    *ratio = sizes_ratio.y;
    center = m_cursor + padding + vec2{ (frame_width - box_size.x / *ratio) / 2.0f, 0.0f };
  }

  boundaries->min = vec2{ m_padding };
  boundaries->max = vec2{ m_padding + m_frame_width };
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

void Debugger::draw_polar_plane(const vec2& origin, const Box& boundaries, Geometry& geo) {
  int increments = 30;
  float angle_increment = MATH_TWO_PI / (float)increments;
  vec4 line_color{ 0.3f, 0.3f, 0.3f, 0.5f };

  for (int i = 0; i < increments; i++) {
    float angle = (float)i * angle_increment;
    draw_polar_line(origin, angle, boundaries, line_color, geo);
  }
}

#endif
