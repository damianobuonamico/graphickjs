#include "freehand_entity.h"
#include "../../input/input_manager.h"
#include "../../../math/models/corner_detector.h"
#include "../../../math/models/path_fitter.h"
#include "../../../math/models/input_parser.h"
#include "../../settings.h"

#ifdef USE_SPRING_FREEHAND

FreehandEntity::FreehandEntity(vec2 position, float pressure, double time)
  : m_transform(TransformComponent{ this, position }), m_points({ {{0.0f, 0.0f, pressure}, time } }) {}

void FreehandEntity::add_point(vec2 position, float pressure, double time) {
  m_points.push_back({ {position.x, position.y, pressure }, time });
}

void FreehandEntity::add_point(vec2 position, float pressure, double time, vec3 updated_data) {
  if (m_points.size() > 1) {
    m_points.back().data = updated_data;
  }

  m_points.push_back({ { position.x, position.y, pressure }, time });
}

void FreehandEntity::tessellate_outline(const vec4& color, RenderingOptions options, Geometry& geo) const {}

size_t FreehandEntity::index_from_t(double t) const {
  for (int i = 1; i < m_points.size(); i++) {
    if (m_points[i].time > t) {
      return i - 1;
    }
  }

  return m_points.size() - 1;
}

Geometry FreehandEntity::tessellate(RenderingOptions options) const {
  size_t points_num = m_points.size();
  Geometry geo;

  if (points_num == 0) return geo;

  vec2 point, direction, normal;
  float width;

  uint32_t offset;

  // TEMP: move to style component
  vec4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
  float stroke_width = 5.0f;
  float sq_stroke_width = std::powf((1.0f + Settings::tessellation_error) * stroke_width / options.zoom, 2.0f);
  vec2 offset_position = m_transform.position().get();

  TessellationParams params = {
    offset_position, options, stroke_width, color, JoinType::Round, CapType::Round, 1.0f,
    false, false, false, false, true, { {0, 0}, {0, 0}, 0 }, { {0, 0}, {0, 0}, 0 }
  };

  params.rendering_options.facet_angle = options.facet_angle / std::sqrtf(stroke_width);
  float facet_angle = params.rendering_options.facet_angle * 0.25f;

  console::log("facet_angle", facet_angle / MATH_PI * 180.0f);

  if (points_num == 1) {
    geo.push_circle(offset_position + XY(m_points[0].data), stroke_width * m_points[0].data.z, color, MATH_TWO_PI / facet_angle);
    return geo;
  } else if (points_num == 2) {
    vec2 p0 = offset_position + XY(m_points[0].data);
    vec2 p1 = offset_position + XY(m_points[1].data);

    float width_start = stroke_width * m_points[0].data.z;
    float width_end = stroke_width * m_points[1].data.z;

    direction = p1 - p0;
    normal = orthogonal(direction);
    normalize(normal, normal);

    vec2 normal_start = normal * width_start;
    vec2 normal_end = normal * width_end;

    tessellate_cap(params, p0, normal_start, false, width_start, geo);

    uint32_t offset = geo.offset();

    geo.push_vertices({
      { p0 - normal_start, color, -width_start }, { p0 + normal_start, color, width_start },
      { p1 - normal_end, color, -width_end }, { p1 + normal_end, color, width_end }
      });
    geo.push_indices({ offset, offset + 1, offset + 2, offset + 2, offset + 3, offset + 1 });

    params.start_join_params.index = offset + 2;

    tessellate_cap(params, p1, normal_end, true, width_end, geo);

    return geo;
  }

  width = stroke_width * m_points[0].data.z;
  point = offset_position + XY(m_points[0].data);
  direction = midpoint(XY(m_points[1].data), XY(m_points[2].data)) - XY(m_points[0].data);
  normal = orthogonal(direction);
  normalize_length(normal, width, normal);

  tessellate_cap(params, point, normal, false, width, geo);

  geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
  offset = geo.offset();

  double time = m_points[m_points.size() - 2].time;
  double time_step = std::min(50.0 / options.zoom, 1.0);

  float stiffness = Settings::spring_constant / Settings::mass_constant;
  float drag = Settings::viscosity_constant;

  vec3 position = m_points[0].data;
  vec3 velocity{ 0.0f };
  vec3 acceleration{ 0.0f };

  vec3 last_position = position;
  float last_width = width;
  size_t last_index = 0;
  int since_last_point = 100;
  int since_last_stroked_point = 100;
  int min_points_interval = (int)(std::max(1.0f / options.zoom, 1.0f) / time_step);

  float theta = std::atan2f(m_points[1].data.y, m_points[1].data.x);
  float new_theta, delta_theta;

  vec3 anchor_start, anchor_end, anchor;

  InstancedGeometry pt_geo;
  pt_geo.push_circle(m_transform.position().get(), 1.0f, vec4{ 0.8f, 0.5f, 0.5f, 1.0f });

  for (double t = m_points[0].time + time_step; t < time; t += time_step) {
    size_t index = index_from_t(t);

    if (index != last_index) {
      zero(velocity);
    }

    anchor_start = m_points[index + 1].data;
    anchor_end = m_points[index + 2].data;

    anchor = lerp(anchor_start, anchor_end, (t - m_points[index].time) / (m_points[index + 1].time - m_points[index].time));

    acceleration = (anchor - position) / stiffness - drag * velocity;
    velocity += acceleration * time_step;
    position += velocity * time_step;

    if (since_last_point > min_points_interval) {
      new_theta = std::atan2f(velocity.y, velocity.x);
      delta_theta = std::fabsf(new_theta - theta);

      // TEMP
      pt_geo.push_instance(XY(position));

      if (delta_theta >= facet_angle || (since_last_stroked_point >= min_points_interval * 10 && squared_distance(position, last_position) > sq_stroke_width)) {
        width = stroke_width * position.z;
        point = offset_position + XY(position);
        normal = orthogonal(XY(velocity));
        normalize_length(normal, width, normal);

        if (delta_theta > params.rendering_options.facet_angle) {
          float angle = std::acosf(dot(normal, params.start_join_params.normal) / (width * width));
          int increments = (int)std::ceilf(angle / params.rendering_options.facet_angle);

          if (increments < 2) {
            geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
            geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
            offset += 2;
          } else {
            uint32_t end_index = offset + increments - 1;
            float increment = angle / (float)increments;
            float bend_direction = dot(XY(position) - XY(last_position), params.start_join_params.normal);
            vec2 bended_normal = params.start_join_params.normal;

            if (bend_direction < 0.0f) {
              increment = -increment;
            }

            for (int i = 1; i < increments; ++i) {
              float angle_offset = (float)i * increment;
              float sin = std::sinf(angle_offset);
              float cos = std::cosf(angle_offset);

              float w = lerp(last_width, width, (float)i / (float)increments);

              vec2 n = {
                bended_normal.x * cos - bended_normal.y * sin,
                bended_normal.x * sin + bended_normal.y * cos
              };

              geo.push_vertices({ { point - n, color, -w }, { point + n, color, w } });
              geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
              offset += 2;
            }

            geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
            geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
            offset += 2;
          }
        } else {
          geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
          geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
          offset += 2;
        }

        theta = new_theta;
        last_position = position;
        last_width = width;
        since_last_stroked_point = -1;

        params.start_join_params.direction = direction;
        params.start_join_params.normal = normal;
        params.start_join_params.index = offset;
      }

      since_last_point = -1;
      since_last_stroked_point++;
    }

    since_last_point++;
    last_index = index;
  }


  for (double t = time; t < m_points.back().time; t += time_step) {
    size_t index = index_from_t(t);

    if (index != last_index) {
      zero(velocity);
    }

    anchor = m_points[index + 1].data;

    acceleration = (anchor - position) / stiffness - drag * velocity;
    velocity += acceleration * time_step;
    position += velocity * time_step;

    if (since_last_point > min_points_interval) {
      new_theta = std::atan2f(velocity.y, velocity.x);
      delta_theta = std::fabsf(new_theta - theta);

      if (delta_theta >= facet_angle || (since_last_stroked_point >= min_points_interval * 10 && squared_distance(position, last_position) > sq_stroke_width)) {
        width = stroke_width * position.z;
        point = offset_position + XY(position);
        normal = orthogonal(XY(velocity));
        normalize_length(normal, width, normal);

        geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
        geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
        offset += 2;

        theta = new_theta;
        last_position = position;
        last_width = width;
        since_last_stroked_point = -1;

        params.start_join_params.direction = direction;
        params.start_join_params.normal = normal;
        params.start_join_params.index = offset;
      }

      since_last_point = -1;
      since_last_stroked_point++;
    }

    since_last_point++;
    last_index = index;
  }



  width = stroke_width * m_points.back().data.z;
  point = offset_position + XY(m_points.back().data);
  direction = XY(m_points.back().data) - midpoint(XY(m_points[points_num - 2].data), XY(m_points[points_num - 3].data));
  normal = orthogonal(direction);
  normalize_length(normal, width, normal);

  geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
  geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
  offset += 2;

  tessellate_cap(params, point, normal, true, width, geo);


  // TEMP
  // Renderer::draw(geo);
  // Renderer::draw(pt_geo);

  return geo;
}

void FreehandEntity::render(RenderingOptions options) const {
#if 1
  Renderer::draw(tessellate(options).wireframe());
  return;
#else
  InstancedGeometry raw_geo;
  raw_geo.push_circle(m_transform.position().get(), 2.0f, vec4{ 0.5f, 0.5f, 0.5f, 1.0f });

  for (auto& point : m_points) {
    raw_geo.push_instance(XY(point.data));
  }

  vec4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
  float width = 5.0f;

  // Renderer::draw(raw_geo);

  InstancedGeometry geo;
  InstancedGeometry velocity_geo;
  InstancedGeometry curvature_geo;
  Geometry tris_geo;



  TessellationParams params = {
    m_transform.position().get(), options,
    width, color,
    JoinType::Round, CapType::Round, 10.0f,
    false, false, true, false, true,
    { vec2{}, vec2{}, 0 }
  };

  geo.push_circle(m_transform.position().get(), 1.0f, vec4{ 0.8f, 0.5f, 0.5f, 1.0f });
  velocity_geo.push_circle(m_transform.position().get(), 0.2f, vec4{ 0.5f, 0.5f, 0.8f, 1.0f });
  curvature_geo.push_circle(m_transform.position().get(), 0.2f, vec4{ 0.5f, 0.8f, 0.5f, 1.0f });

  if (m_points.size() < 3) return;

  double time = m_points[m_points.size() - 2].time;
  double time_step = std::min(50.0 / options.zoom, 1.0);

  vec3 s{ 0.0f, 0.0f, m_points[0].data.z };
  vec3 v{ 0.0f };
  vec3 a{ 0.0f };

  float M = Settings::spring_constant / Settings::mass_constant;
  float k = Settings::viscosity_constant;

  uint last_index = 0;
  int since_last_pt = 100;

  vec2 pos = m_transform.position().get();

  vec2 normal = normalize(orthogonal(XY(m_points[1].data))) * width * s.z;
  // tessellate_cap(params, pos, normal, false, width * s.z, tris_geo);
  uint32_t offset = tris_geo.offset();

  tris_geo.push_vertices({ { pos - normal, color, -width * s.z }, { pos + normal, color, width * s.z } });
  offset += 2;

  vec2 last_point{ 0.0f };
  vec2 last_last_point{ 0.0f };

  double time_start = m_points.front().time;

  float theta = std::atan2(m_points[1].data.y, m_points[1].data.x);

  for (double t = m_points.front().time + time_step; t < time; t += time_step) {
    uint index = index_from_t(t);

    if (last_index != index) {
      v = { 0.0f, 0.0f, 0.0f };
    }

    vec3 anchor_start = { m_points[index + 1].data.x, m_points[index + 1].data.y, m_points[index + 1].data.z };
    vec3 anchor_end = { m_points[index + 2].data.x, m_points[index + 2].data.y, m_points[index + 2].data.z };

    vec3 anchor = lerp(anchor_start, anchor_end, (t - m_points[index].time) / (m_points[index + 1].time - m_points[index].time));

    a = (anchor - s) / M - k * v;
    v += a * time_step;
    s += v * time_step;


    if (since_last_pt > std::min(1.0 / options.zoom, 1.0) / time_step) {
      geo.push_instance({ s.x, s.y });

      vec2 d1 = normalize(last_point - last_last_point);
      vec2 d2 = normalize(vec2{ s.x, s.y } - last_point);


      float new_theta = std::atan2(v.y, v.x);
      float d_theta = std::abs(new_theta - theta);

      if (d_theta >= Settings::facet_angle) {
        theta = new_theta;
        curvature_geo.push_instance({ s.x, s.y });

        vec2 normal = orthogonal(d2) * width * s.z;
        vec2 direction = vec2{ s.x, s.y } - last_point;

        // if (angle > Settings::corners_angle_threshold && t > time_start + 2 * time_step) {
        //   tessellate_join(params, pos + vec2{ s.x, s.y }, direction, normal, width* s.z, nullptr, tris_geo);
        //   offset = tris_geo.offset();
        // }

        tris_geo.push_vertices({ { pos + vec2{ s.x, s.y } - normal, color, -width * s.z }, { pos + vec2{ s.x, s.y } + normal, color, width * s.z } });
        tris_geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
        offset += 2;

        params.start_join_params.direction = direction;
        params.start_join_params.normal = normal;
        params.start_join_params.index = offset;

        velocity_geo.push_instance(vec2{ s.x, s.y });
        last_last_point = last_point;
        last_point = vec2{ s.x, s.y };
      }

      since_last_pt = -1;
    }

    since_last_pt++;
    last_index = index;
  }

  for (double t = time; t <= m_points.back().time; t += time_step) {
    uint index = index_from_t(t);

    if (last_index != index) {
      v = { 0.0f, 0.0f, 0.0f };
    }

    vec3 anchor = { m_points[index + 1].data.x, m_points[index + 1].data.y, m_points[index + 1].data.z };

    a = (anchor - s) / M - k * v;
    v += a * time_step;
    s += v * time_step;

    if (since_last_pt > std::min(1.0 / options.zoom, 1.0) / time_step) {
      geo.push_instance({ s.x, s.y });

      vec2 d1 = normalize(last_point - last_last_point);
      vec2 d2 = normalize(vec2{ s.x, s.y } - last_point);

      float angle = std::acos(dot(d1, d2));

      if (angle >= Settings::facet_angle) {
        vec2 normal = orthogonal(d2) * width * s.z;
        // vec2 normal = normalize(orthogonal(midpoint(d1, d2))) * width;
        vec2 direction = vec2{ s.x, s.y } - last_point;

        if (angle > Settings::corners_angle_threshold && t > time_start + 2 * time_step) {
          // tessellate_join(params, pos + vec2{ s.x, s.y }, direction, normal, width* s.z, nullptr, tris_geo);
          offset = tris_geo.offset();
        }

        tris_geo.push_vertices({ { pos + vec2{ s.x, s.y } - normal, color, -width * s.z }, { pos + vec2{ s.x, s.y } + normal, color, width * s.z } });
        tris_geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });
        offset += 2;

        params.start_join_params.direction = direction;
        params.start_join_params.normal = normal;
        params.start_join_params.index = offset;

        velocity_geo.push_instance(vec2{ s.x, s.y });
        last_last_point = last_point;
        last_point = vec2{ s.x, s.y };
      }

      since_last_pt = -1;
    }

    since_last_pt++;
    last_index = index;
  }

  // for (auto& pt : tess_points) {
  //   curvature_geo.push_instance(pt.position + pt.normal * pt.curvature * 10);
  // }

  geo.push_instance(XY(m_points.back().data));
  velocity_geo.push_instance(XY(m_points.back().data));

  vec2 point = pos + XY(m_points.back().data);
  vec2 direction = point - XY(m_points[m_points.size() - 2].data) - pos;
  normal = orthogonal(direction);
  normalize_length(normal, width * m_points.back().data.z, normal);

  tris_geo.push_vertices({ { point - normal, params.color, -width * m_points.back().data.z }, { point + normal, params.color, width * m_points.back().data.z } });
  tris_geo.push_indices({ offset - 2, offset - 1, offset, offset, offset + 1, offset - 1 });

  params.start_join_params.direction = direction;
  params.start_join_params.normal = normal;
  params.start_join_params.index = offset;

  // tessellate_cap(params, point, normal, true, width * m_points.back().pressure, tris_geo);

  Renderer::draw(tris_geo);
  // Renderer::draw(geo);
  // Renderer::draw(velocity_geo);
  // Renderer::draw(curvature_geo);
  Renderer::draw(tessellate(options).wireframe());
#endif
}

#else

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

#endif