#pragma once

#include "../math/vec2.h"
#include "../math/box.h"

class Viewport {
public:
  Viewport() = default;
  Viewport(const vec2& position, float zoom, float rotation);
  Viewport(const Viewport&) = delete;
  Viewport(Viewport&&) = delete;

  ~Viewport() = default;

  inline const vec2& position() const { return m_position; };
  inline const float zoom() const { return m_zoom; };

  void resize(const vec2& size, const vec2& offset);

  void move_to(const vec2& position);
  void zoom_to(float zoom);
  void zoom_to(float zoom, const vec2& zoom_origin);

  void set_bounds(const Box& bounds);
  bool is_visible(const Box& box);

  // TODO: JSON

  vec2 client_to_scene(const vec2& position);
  vec2 scene_to_client(const vec2& position);
private:
  vec2 client_to_scene(const vec2& position, float zoom_override);
  vec2 scene_to_client(const vec2& position, float zoom_override);
private:
  vec2 m_size;
  vec2 m_offset;

  vec2 m_position;
  float m_zoom;
  float m_rotation;

  vec2 m_min_position;
  vec2 m_max_position;
  float m_min_zoom;
};
