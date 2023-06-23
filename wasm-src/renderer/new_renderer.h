#pragma once

#include "tiler.h"
#include "renderer_data.h"

#include "gpu/shaders.h"

#include "../utils/uuid.h"

// TEMP
#include "geometry/geometry.h"

namespace Graphick::Render {

  class Renderer {
  public:
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    static inline Renderer* get() { return s_instance; }

    static void init();
    static void shutdown();

    static void resize(const ivec2 size, float dpr);

    static void begin_frame(const vec2 position, float zoom);
    static void end_frame();

    static void draw(const Geometry::Path& path, const vec4& color);
    static void draw(const Temp::Geo& geo);
  private:
    Renderer();
    ~Renderer() = default;

    void draw_spans();
    void draw_tiles();

    // @deprecated
    void draw_fills();
    void draw_masks();
    void draw_lines();
  private:
    Viewport m_viewport;
    Tiler m_tiler;
    // TEMP
    Temp::Geo m_lines;
    UUID m_framebuffer_id;

    GPU::Programs m_programs;
  private:
    static Renderer* s_instance;
  };

}
