#pragma once

#include "../../math/vec2.h"
#include "../../math/box.h"
#include "../../io/json/json.h"

class Viewport {
public:
  Viewport();
  Viewport(const vec2& position, float zoom, float rotation);
  Viewport(const Viewport&) = default;
  Viewport(Viewport&&) = default;

  ~Viewport() = default;

  inline const vec2 position() const { return m_position; }
  inline const float zoom() const { return m_zoom; }
  inline const vec2 size() const { return m_size; }
  inline Box visible() const { return { -m_position, m_size / m_zoom - m_position }; }

  void resize(const vec2& size, const vec2& offset);

  void move(const vec2& movement);
  void move_to(const vec2& position);
  void zoom_to(float zoom);
  void zoom_to(float zoom, const vec2& zoom_origin);

  void set_bounds(const Box& bounds);
  bool is_visible(const Box& box);

  void load(const JSON& data);
  JSON json() const;

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

  vec2 m_min_position = std::numeric_limits<vec2>::min();
  vec2 m_max_position = std::numeric_limits<vec2>::max();
  float m_min_zoom{ 0.01f };
};
