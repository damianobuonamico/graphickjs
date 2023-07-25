#pragma once

#include "tiler.h"
#include "gpu/shaders.h"

#include "../math/ivec2.h"
#include "../math/vec2.h"
#include "../math/mat4.h"

#include "../utils/uuid.h"

namespace Graphick::Renderer {

  namespace Geometry {
    class Path;
  }

  class Renderer {
  public:
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    static void init();
    static void shutdown();

    static void begin_frame(const Viewport& viewport);
    static void end_frame();

    static void draw(const Geometry::Path& path, const vec4& color = { 0.0f, 0.0f, 0.0f, 1.0f });
    static void draw_outline(const Geometry::Path& path);
  private:
    Renderer() = default;
    ~Renderer() = default;

    static inline Renderer* get() { return s_instance; }

    void draw_opaque_tiles();
    void draw_masked_tiles();

    void init_batched_lines_renderer();
    void begin_lines_batch();
    void add_to_lines_batch(const Geometry::Path& path);
    void flush_lines_batch();
  private:
    GPU::Programs m_programs;

    mat4 m_projection;
    mat4 m_translation;
    mat4 m_tiles_projection;
    mat4 m_tiles_translation;

    BatchedLinesData m_lines_data;

    uuid m_quad_vertex_positions_buffer_id = 0;
    uuid m_quad_vertex_indices_buffer_id = 0;
    uuid m_masks_texture_id = 0;

    // Replace with render state / render options
    Viewport m_viewport;
    Tiler m_tiler;
  private:
    static Renderer* s_instance;
  };

}
