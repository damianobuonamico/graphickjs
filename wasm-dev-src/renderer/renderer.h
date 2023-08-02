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
    void draw_masked_tiles_batch(const std::vector<MaskedTile> tiles, const size_t i, const std::vector<uint8_t*> textures);

    void init_instanced_renderers();
    void begin_instanced_renderers();

    void add_line_instances(const Geometry::Path& path);
    void add_linear_segment_instance(const vec2 p0, const vec2 p3);
    void add_cubic_segment_instance(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3);
    void add_vertex_instances(const Geometry::Path& path);
    void add_square_instance(const vec2 position);
    void add_circle_instance(const vec2 position);

    void flush_line_instances();
    void flush_square_instances();
    void flush_circle_instances();

    void ensure_instance_buffer_size(InstancedMeshData& data);
  private:
    GPU::Programs m_programs;

    mat4 m_projection;
    mat4 m_translation;
    mat4 m_tiles_projection;
    mat4 m_tiles_translation;

    // TODO: Instance attributes
    InstancedLinesData m_lines_data;
    InstancedMeshData m_square_data = { "square" };
    InstancedMeshData m_circle_data = { "circle" };

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
