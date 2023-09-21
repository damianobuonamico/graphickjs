#pragma once

#include "tiler.h"
#include "gpu/shaders.h"

#include "../math/ivec2.h"
#include "../math/vec2.h"
#include "../math/mat4.h"

#include "../utils/uuid.h"

#include "../lib/stb/stb_truetype.h"

namespace Graphick::Renderer {

  namespace Geometry {
    class Path;
  }
  namespace Geometry::Internal {
    class PathInternal;
  }

  class Renderer {
  public:
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    static void init();
    static void shutdown();

    static void begin_frame(const Viewport& viewport);
    static void end_frame();

    static void draw(const Geometry::Path& path, const float z_index, const vec2 translation = { 0.0f, 0.0f }, const vec4& color = { 0.0f, 0.0f, 0.0f, 1.0f });
    static void draw_outline(const uuid id, const Geometry::Path& path, const vec2 translation);
    static void draw_outline(const Geometry::Internal::PathInternal& path, const vec2 translation);

    static void debug_rect(const Math::rect rect, const vec4& color = { 0.0f, 0.0f, 0.0f, 0.5f });
    static void debug_text(const std::string& text, const vec2 position, const vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
  private:
    Renderer() = default;
    ~Renderer() = default;

    static inline Renderer* get() { return s_instance; }

    void draw_opaque_tiles();
    void draw_masked_tiles();
    void draw_masked_tiles_batch(const std::vector<MaskedTile> tiles, const size_t i, const std::vector<uint8_t*> textures);

    void init_instanced_renderers();
    void begin_instanced_renderers();

    void add_line_instances(const Geometry::Path& path, const vec2 translation);
    void add_line_instances(const Geometry::Internal::PathInternal& path, const vec2 translation);
    void add_linear_segment_instance(const vec2 p0, const vec2 p3);
    void add_cubic_segment_instance(const vec2 p0, const vec2 p1, const vec2 p2, const vec2 p3);
    void add_vertex_instances(const uuid id, const Geometry::Path& path, const vec2 translation);
    void add_square_instance(const vec2 position);
    void add_white_square_instance(const vec2 position);
    void add_circle_instance(const vec2 position);

    void flush_line_instances();
    void flush_generic_square_instances(InstancedMeshData& data, const vec4& color, const float size);
    void flush_square_instances();
    void flush_white_square_instances();
    void flush_circle_instances();

    void ensure_instance_buffer_size(InstancedMeshData& data);

    void init_text_renderer();
  private:
    GPU::Programs m_programs;

    mat4 m_projection;
    mat4 m_translation;
    mat4 m_tiles_projection;
    mat4 m_tiles_translation;

    CommonData m_common_data;

    InstancedLinesData m_lines_data;
    InstancedMeshData m_square_data = { "square" };
    InstancedMeshData m_white_square_data = { "white_square" };
    InstancedMeshData m_circle_data = { "circle" };

    uuid m_quad_vertex_positions_buffer_id = 0;
    uuid m_quad_vertex_indices_buffer_id = 0;
    uuid m_masks_texture_id = 0;

    // Replace with render state / render options
    Viewport m_viewport;
    Tiler m_tiler;

    unsigned char m_bitmap[128 * 128];
    stbtt_bakedchar m_cdata[96]; // ASCII 32..126 is 95 glyphs
    uuid m_debug_font_atlas_id = 0;
    uuid m_debug_vertex_buffer_id = 0;
  private:
    static Renderer* s_instance;
  };

}
