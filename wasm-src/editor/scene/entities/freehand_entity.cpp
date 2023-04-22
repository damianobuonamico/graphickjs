#include "freehand_entity.h"

#include "../../../math/math.h"
#include "../../../renderer/geometry/stroker.h"
#include "../../settings.h"

FreehandEntity::FreehandEntity(vec2 position, float pressure, double time)
  : Entity(CategorySelectable), m_transform({ this, position }), m_points({ {{0.0f, 0.0f, pressure}, time } }) {}

FreehandEntity::FreehandEntity(const JSON& data)
  : Entity(CategorySelectable), m_transform({ this, data.has("transform") ? data.at("transform") : JSON() }), m_points() {
  if (!data.has("type") || data.at("type").to_string() != "freehand") {
    console::error("Invalid entity type", data.at("type").to_string());
    return;
  }

  if (data.has("points")) {
    for (const auto& point : data.at("points").array_range()) {
      vec4 point_data = point.to_vec4();
      m_points.push_back({ XYZ(point_data), (double)point_data.w });
    }
  }
}

void FreehandEntity::add_point(vec2 position, float pressure, double time) {
  m_points.push_back({ {position.x, position.y, pressure }, time });
}

void FreehandEntity::add_point(vec2 position, float pressure, double time, vec3 updated_data) {
  if (m_points.size() > 1) {
    m_points.back().data = updated_data;
  }

  m_points.push_back({ { position.x, position.y, pressure }, time });
}

void FreehandEntity::tessellate_outline(const vec4& color, const RenderingOptions& options, Geometry& geo) const {
  size_t points_num = m_points.size();

  vec2 point;

  float stroke_width = 5.0f;
  float sq_stroke_width = std::powf((1.0f + Settings::tessellation_error) * stroke_width / options.zoom, 2.0f);
  vec2 offset_position = m_transform.position().get();

  float facet_angle = options.facet_angle * 0.25f;

  if (points_num == 1) {
    geo.push_line(offset_position - 0.01f, offset_position + 0.01f);
    return;
  } else if (points_num == 2) {
    geo.push_line(offset_position + XY(m_points[0].data), offset_position + XY(m_points[1].data), color);
    return;
  }

  uint32_t offset = geo.offset();

  {
    point = offset_position + XY(m_points[0].data);
    // geo.push_vertex({ point, color });
    geo.push_vertex({ point });
    offset++;
  }

  double time = m_points[m_points.size() - 2].time;
  double time_step = std::min(1.5, std::round((0.1 + 1 / options.zoom) * 10) / 10);

  float stiffness = Settings::spring_constant / Settings::mass_constant;
  float drag = Settings::viscosity_constant;

  vec3 position = m_points[0].data;
  vec3 velocity{ 0.0f };
  vec3 acceleration{ 0.0f };

  vec3 last_position = position;
  size_t last_index = 0;
  int since_last_point = 100;
  int since_last_stroked_point = 100;
  int min_points_interval = (int)(std::max(1.0f / options.zoom, 1.0f) / time_step);

  float theta = std::atan2f(m_points[1].data.y, m_points[1].data.x);
  float new_theta, delta_theta;

  vec3 anchor_start, anchor_end, anchor;

  for (double t = m_points[0].time + time_step; t < time; t += time_step) {
    size_t index = index_from_t(t);

    if (index != last_index) {
      zero(velocity);
    }

    anchor_start = m_points[index + 1].data;
    anchor_end = m_points[index + 2].data;

    anchor = lerp(anchor_start, anchor_end, (float)(t - m_points[index].time) / (float)(m_points[index + 1].time - m_points[index].time));

    acceleration.x = (anchor.x - position.x) / stiffness - drag * velocity.x;
    acceleration.y = (anchor.y - position.y) / stiffness - drag * velocity.y;

    velocity += acceleration * (float)time_step;
    position += velocity * (float)time_step;

    if (since_last_point > min_points_interval) {
      new_theta = std::atan2f(velocity.y, velocity.x);
      delta_theta = std::fabsf(new_theta - theta);

      if (delta_theta >= facet_angle || (since_last_stroked_point >= min_points_interval * 10 && squared_distance(position, last_position) > sq_stroke_width)) {
        point = offset_position + XY(position);

        // geo.push_vertex({ point, color });
        geo.push_vertex({ point });
        geo.push_indices({ offset - 1, offset });
        offset++;

        theta = new_theta;
        last_position = position;
        since_last_stroked_point = -1;
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

    acceleration.x = (anchor.x - position.x) / stiffness - drag * velocity.x;
    acceleration.y = (anchor.y - position.y) / stiffness - drag * velocity.y;

    velocity += acceleration * (float)time_step;
    position += velocity * (float)time_step;

    if (since_last_point > min_points_interval) {
      new_theta = std::atan2f(velocity.y, velocity.x);
      delta_theta = std::fabsf(new_theta - theta);

      if (delta_theta >= facet_angle || (since_last_stroked_point >= min_points_interval * 10 && squared_distance(position, last_position) > sq_stroke_width)) {
        point = offset_position + XY(position);

        // geo.push_vertex({ point, color });
        geo.push_vertex({ point });
        geo.push_indices({ offset - 1, offset });
        offset++;

        theta = new_theta;
        last_position = position;
        since_last_stroked_point = -1;
      }

      since_last_point = -1;
      since_last_stroked_point++;
    }

    since_last_point++;
    last_index = index;
  }

  {
    point = offset_position + XY(m_points.back().data);

    // geo.push_vertex({ point, color });
    geo.push_vertex({ point });
    geo.push_indices({ offset - 1, offset });
    offset++;
  }
}

void FreehandEntity::render(const RenderingOptions& options) const {
  if (m_points.empty()) return;

  // TODO: replace with stroke width
  Box box = m_transform.bounding_box();
  box.min -= 5.0f;
  box.max += 5.0f;

  if (!does_box_intersect_box(box, options.viewport)) return;

  // TODO: do not recalculate geometry if only position changed
  vec2 position = m_transform.position().get();
  UUID id{ (uint32_t)m_points.size(), (uint32_t)std::round(options.facet_angle * 100), (uint32_t)(std::abs(position.x) * 10000.0f), (uint32_t)(std::abs(position.y) * 10000.0f) };

  if (m_geometry.id() == id) {
    Renderer::draw(m_geometry.get());
    return;
  }

  Geometry geo = tessellate(options);
  m_geometry.set(geo, id);

  Renderer::draw(geo);
}

Entity* FreehandEntity::entity_at(const vec2& position, bool lower_level, float threshold) {
  if (m_points.empty()) return nullptr;

  vec2 pos = position - m_transform.position().get();
  Box box = m_transform.bounding_box();

  if (is_point_in_box(position, box)) {
    return this;
  }

  return nullptr;
}

void FreehandEntity::entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) {
  if (m_points.empty()) return;

  Box entity_box = m_transform.bounding_box();

  if (does_box_intersect_box(box, entity_box)) {
    entities.push_back(this);
  }
}

JSON FreehandEntity::json() const {
  JSON object = JSON::object();
  JSON points = JSON::array();

  for (const auto& point : m_points) {
    points.append(JSON::array(point.data.x, point.data.y, point.data.z, (float)point.time));
  }

  object["type"] = "freehand";
  object["points"] = points;
  object["transform"] = m_transform.json();

  return object;
}

size_t FreehandEntity::index_from_t(double t) const {
  for (int i = 1; i < m_points.size(); i++) {
    if (m_points[i].time > t) {
      return i - 1;
    }
  }

  return m_points.size() - 1;
}

Geometry FreehandEntity::tessellate(const RenderingOptions& options) const {
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

  if (points_num == 1) {
    geo.push_circle(offset_position + XY(m_points[0].data), stroke_width * m_points[0].data.z, color, (uint32_t)(MATH_TWO_PI / facet_angle));
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

    // geo.push_vertices({
    //   { p0 - normal_start, color, -width_start }, { p0 + normal_start, color, width_start },
    //   { p1 - normal_end, color, -width_end }, { p1 + normal_end, color, width_end }
    //   });
    geo.push_vertices({
      { p0 - normal_start }, { p0 + normal_start },
      { p1 - normal_end }, { p1 + normal_end }
      });
    geo.push_indices({ offset, offset + 1, offset + 2, offset + 2, offset + 3, offset + 1 });

    params.start_join_params.index = offset + 2;

    tessellate_cap(params, p1, normal_end, true, width_end, geo);

    return geo;
  }

  {
    width = stroke_width * m_points[0].data.z;
    point = offset_position + XY(m_points[0].data);
    direction = midpoint(XY(m_points[1].data), XY(m_points[2].data)) - XY(m_points[0].data);
    normal = orthogonal(direction);
    normalize_length(normal, width, normal);

    tessellate_cap(params, point, normal, false, width, geo);

    // geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
    geo.push_vertices({ { point - normal }, { point + normal } });
    offset = geo.offset();
  }

  double time = m_points[m_points.size() - 2].time;
  double time_step = std::min(1.5, std::round((0.1 + 1 / options.zoom) * 10) / 10);

  float stiffness = Settings::spring_constant / Settings::mass_constant;
  float pressure_stiffness = stiffness * 10.0f;
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

  for (double t = m_points[0].time + time_step; t < time; t += time_step) {
    size_t index = index_from_t(t);

    if (index != last_index) {
      zero(velocity);
    }

    anchor_start = m_points[index + 1].data;
    anchor_end = m_points[index + 2].data;

    anchor = lerp(anchor_start, anchor_end, (float)(t - m_points[index].time) / (float)(m_points[index + 1].time - m_points[index].time));

    acceleration.x = (anchor.x - position.x) / stiffness - drag * velocity.x;
    acceleration.y = (anchor.y - position.y) / stiffness - drag * velocity.y;
    acceleration.z = (anchor.z - position.z) / pressure_stiffness - drag * velocity.z;

    velocity += acceleration * (float)time_step;
    position += velocity * (float)time_step;

    if (since_last_point > min_points_interval) {
      new_theta = std::atan2f(velocity.y, velocity.x);
      delta_theta = std::fabsf(new_theta - theta);

      if (delta_theta >= facet_angle || (since_last_stroked_point >= min_points_interval * 10 && squared_distance(position, last_position) > sq_stroke_width)) {
        width = stroke_width * position.z;
        point = offset_position + XY(position);
        normal = orthogonal(XY(velocity));
        normalize_length(normal, width, normal);

        if (delta_theta > params.rendering_options.facet_angle) {
          float angle = std::acosf(dot(normal, params.start_join_params.normal) / (width * width));
          if (is_almost_zero(angle)) {
            angle = MATH_PI;
          }

          int increments = (int)std::ceilf(angle / params.rendering_options.facet_angle);

          if (increments < 2) {
            // geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
            geo.push_vertices({ { point - normal }, { point + normal } });
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

              // geo.push_vertices({ { point - n, color, -w }, { point + n, color, w } });
              geo.push_vertices({ { point - n }, { point + n } });
              geo.push_indices({ offset - 2, offset - 1, offset, offset - 1, offset + 1, offset });
              offset += 2;
            }

            // geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
            geo.push_vertices({ { point - normal }, { point + normal } });
            geo.push_indices({ offset - 2, offset - 1, offset, offset - 1, offset + 1, offset });
            offset += 2;
          }
        } else {
          // geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
          geo.push_vertices({ { point - normal }, { point + normal } });
          geo.push_indices({ offset - 2, offset - 1, offset, offset - 1 , offset + 1, offset });
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

    acceleration.x = (anchor.x - position.x) / stiffness - drag * velocity.x;
    acceleration.y = (anchor.y - position.y) / stiffness - drag * velocity.y;
    acceleration.z = (anchor.z - position.z) / pressure_stiffness - drag * velocity.z;

    velocity += acceleration * (float)time_step;
    position += velocity * (float)time_step;

    if (since_last_point > min_points_interval) {
      new_theta = std::atan2f(velocity.y, velocity.x);
      delta_theta = std::fabsf(new_theta - theta);

      if (delta_theta >= facet_angle || (since_last_stroked_point >= min_points_interval * 10 && squared_distance(position, last_position) > sq_stroke_width)) {
        width = stroke_width * position.z;
        point = offset_position + XY(position);
        normal = orthogonal(XY(velocity));
        normalize_length(normal, width, normal);

        // geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
        geo.push_vertices({ { point - normal }, { point + normal } });
        geo.push_indices({ offset - 2, offset - 1, offset, offset - 1, offset + 1, offset });
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

  {
    width = stroke_width * last_position.z;
    point = offset_position + XY(m_points.back().data);
    direction = XY(m_points.back().data) - midpoint(XY(m_points[points_num - 2].data), XY(m_points[points_num - 3].data));
    normal = orthogonal(direction);
    normalize_length(normal, width, normal);

    // geo.push_vertices({ { point - normal, color, -width }, { point + normal, color, width } });
    geo.push_vertices({ { point - normal }, { point + normal } });
    geo.push_indices({ offset - 2, offset - 1, offset, offset - 1, offset + 1, offset });
    offset += 2;

    tessellate_cap(params, point, normal, true, width, geo);
  }

  return geo;
}
