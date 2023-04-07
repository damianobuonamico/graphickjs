#include "viewport.h"

#include "../../math/math.h"
#include "../../math/vector.h"
#include "../../utils/defines.h"
#include "../../utils/console.h"

Viewport::Viewport()
  : m_position({ 0.0f, 0.0f }), m_zoom(1.0f), m_rotation(0.0f) {}

Viewport::Viewport(const vec2& position, float zoom, float rotation)
  : m_position(position), m_zoom(zoom), m_rotation(rotation) {}

void Viewport::resize(const vec2& size, const vec2& offset) {
  m_size = size;
  m_offset = offset;
}

void Viewport::move(const vec2& movement) {
  move_to(m_position + movement);
}

void Viewport::move_to(const vec2& position) {
  if (
    m_min_position == std::numeric_limits<vec2>::min() &&
    m_max_position == std::numeric_limits<vec2>::max()
    ) {
    m_position = position;
    return;
  }

  vec2 min_position = (m_size - m_max_position * m_zoom) / m_zoom;
  vec2 max_position = m_min_position;

  if (m_max_position.x * m_zoom < m_size.x) {
    max_position.x = -(m_max_position.x * m_zoom - m_size.x) / (2 * m_zoom);
  }

  if (m_max_position.y * m_zoom < m_size.y) {
    max_position.y = -(m_max_position.y * m_zoom - m_size.y) / (2 * m_zoom);
  }

  m_position = min(max(position, min_position), max_position);
}

void Viewport::zoom_to(float zoom) {
  m_zoom = round(std::clamp(zoom, std::max(m_min_zoom, ZOOM_MIN), ZOOM_MAX), 0.0001f);
}

void Viewport::zoom_to(float zoom, const vec2& zoom_origin) {
  float zoom_value = round(std::clamp(zoom, std::max(m_min_zoom, ZOOM_MIN), ZOOM_MAX), 0.0001f);

  vec2 position_delta = client_to_scene(zoom_origin, zoom_value) - client_to_scene(zoom_origin);

  m_zoom = zoom_value;
  move(position_delta);
}

void Viewport::set_bounds(const Box& bounds) {
  m_min_position = bounds.min;
  m_max_position = bounds.max;

  vec2 bounds_size = bounds.max - bounds.min;

  if (bounds_size.x > bounds_size.y) {
    m_min_zoom = m_size.x / bounds_size.x;
  } else {
    m_min_zoom = m_size.y / bounds_size.y;
  }
}

bool Viewport::is_visible(const Box& box) {
  return does_box_intersect_box(box, visible());
}

void Viewport::json(std::stringstream& ss) const {
  ss << "{\"position\":" << stringify(m_position);
  ss << ",\"zoom\":" << m_zoom;
  ss << ",\"rotation\":" << m_rotation;
  ss << ",\"min_position\":" << stringify(m_min_position);
  ss << ",\"max_position\":" << stringify(m_max_position);
  ss << ",\"min_zoom\":" << m_min_zoom << "}";
}

vec2 Viewport::client_to_scene(const vec2& position) {
  return (position - m_offset) / m_zoom - m_position;
}

vec2 Viewport::scene_to_client(const vec2& position) {
  return (position + m_position) * m_zoom + m_offset;
}

vec2 Viewport::client_to_scene(const vec2& position, float zoom_override) {
  return (position - m_offset) / zoom_override - m_position;
}

vec2 Viewport::scene_to_client(const vec2& position, float zoom_override) {
  return (position + m_position) * zoom_override + m_offset;
}
