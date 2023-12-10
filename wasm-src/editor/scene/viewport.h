#pragma once

#include "../../math/vec2.h"
#include "../../math/rect.h"

namespace Graphick::Editor {

  class Viewport {
  public:
    Viewport();
    Viewport(const vec2 position, float zoom, float rotation);

    ~Viewport() = default;

    inline vec2 position() const { return m_position; }
    inline float zoom() const { return m_zoom; }
    inline ivec2 size() const { return m_size; }
    inline float dpr() const { return m_dpr; }
    inline rect visible() const { return { -m_position, vec2{ (float)m_size.x, (float)m_size.y } / m_zoom - m_position }; }

    void resize(const ivec2 size, const ivec2 offset, float dpr);

    void move(const vec2 movement);
    void move_to(const vec2 position);
    void zoom_to(float zoom);
    void zoom_to(float zoom, const vec2 zoom_origin);

    void set_bounds(const rect& bounds);
    bool is_visible(const rect& rect);

    // void load(const JSON& data);
    // JSON json() const;

    vec2 client_to_scene(const vec2 position);
    vec2 scene_to_client(const vec2 position);
  private:
    vec2 client_to_scene(const vec2 position, float zoom_override);
    vec2 scene_to_client(const vec2 position, float zoom_override);
  private:
    ivec2 m_size;
    ivec2 m_offset;
    float m_dpr;

    vec2 m_position;
    float m_zoom;
    float m_rotation;

    vec2 m_min_position = std::numeric_limits<vec2>::lowest();
    vec2 m_max_position = std::numeric_limits<vec2>::max();
    float m_min_zoom{ 0.01f };
  };

}
