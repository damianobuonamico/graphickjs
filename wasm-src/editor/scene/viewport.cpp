/**
 * @file viewport.cpp
 * @brief This file contains the implementation of the Viewport class.
 */

#include "viewport.h"

#include "../../math/math.h"
#include "../../math/scalar.h"
#include "../../math/vector.h"

#include "../../geom/intersections.h"

#include "../../utils/console.h"
#include "../../utils/defines.h"

namespace graphick::editor {

Viewport::Viewport() : m_position({0.0f, 0.0f}), m_zoom(1.0f), m_rotation(0.0f), m_size({0, 0}) { }

Viewport::Viewport(const vec2 position, float zoom, float rotation) :
  m_position(position), m_zoom(zoom), m_rotation(rotation), m_size({0, 0}) { }

void Viewport::resize(const ivec2 size, const ivec2 offset, float dpr) {
  m_size = size;
  m_offset = offset;
  m_dpr = dpr;
}

void Viewport::move(const vec2 movement) { move_to(m_position + movement); }

void Viewport::move_to(const vec2 position) {
  if (m_min_position == std::numeric_limits<vec2>::lowest() && m_max_position == std::numeric_limits<vec2>::max()) {
    m_position = position;
    return;
  }

  vec2 min_position = (vec2(m_size) - m_max_position * m_zoom) / m_zoom;
  vec2 max_position = m_min_position;

  if (m_max_position.x * m_zoom < m_size.x) {
    max_position.x = -(m_max_position.x * m_zoom - m_size.x) / (2 * m_zoom);
  }

  if (m_max_position.y * m_zoom < m_size.y) {
    max_position.y = -(m_max_position.y * m_zoom - m_size.y) / (2 * m_zoom);
  }

  m_position = math::min(math::max(position, min_position), max_position);
}

void Viewport::zoom_to(float zoom) { m_zoom = math::round(math::clamp(zoom, std::max(m_min_zoom, ZOOM_MIN), ZOOM_MAX), 0.0001f); }

void Viewport::zoom_to(float zoom, const vec2 zoom_origin) {
  float zoom_value = math::round(math::clamp(zoom, std::max(m_min_zoom, ZOOM_MIN), ZOOM_MAX), 0.0001f);

  vec2 position_delta = client_to_scene(zoom_origin, zoom_value) - client_to_scene(zoom_origin);

  m_zoom = zoom_value;
  move(position_delta);
}

void Viewport::set_bounds(const rect& bounds) {
  m_min_position = bounds.min;
  m_max_position = bounds.max;

  vec2 bounds_size = bounds.max - bounds.min;

  if (bounds_size.x > bounds_size.y) {
    m_min_zoom = m_size.x / bounds_size.x;
  } else {
    m_min_zoom = m_size.y / bounds_size.y;
  }
}

bool Viewport::is_visible(const rect& rect) { return geom::does_rect_intersect_rect(rect, visible()); }

vec2 Viewport::client_to_scene(const vec2 position) { return (position - vec2(m_offset)) / m_zoom - m_position; }

vec2 Viewport::scene_to_client(const vec2 position) { return (position + m_position) * m_zoom + vec2(m_offset); }

vec2 Viewport::client_to_scene(const vec2 position, float zoom_override) {
  return (position - vec2(m_offset)) / zoom_override - m_position;
}

vec2 Viewport::scene_to_client(const vec2 position, float zoom_override) {
  return (position + m_position) * zoom_override + vec2(m_offset);
}

}  // namespace graphick::editor
