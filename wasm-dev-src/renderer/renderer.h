#pragma once

#include "renderer_data.h"
#include "gpu/shaders.h"

#include "../utils/uuid.h"

#include "../lib/blaze/src/DestinationImage.h"

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

    static void render_frame(const Viewport& viewport);

    // TODO: Move to IO
    static void upload_vector_image(const uint8_t* ptr, const int size);

    // TEMP
    static void begin_frame(const Viewport& viewport);
    static void end_frame();

    static void draw(const Geometry::Path& path);
    // TODO: Batch outline draw calls
    static void draw_outline(const Geometry::Path& path) {};
  private:
    Renderer() = default;
    ~Renderer() = default;

    static inline Renderer* get() { return s_instance; }

    void update_image_data();
    void render_frame_backend();

    // TODO: Move to viewport
    Blaze::Matrix get_matrix();
  private:
    GPU::Programs m_programs;

#ifdef EMSCRIPTEN
    int m_ctx;
#endif

    uuid m_quad_vertex_positions_buffer_id = 0;
    uuid m_quad_vertex_indices_buffer_id = 0;
    uuid m_frame_texture_id = 0;

    std::vector<std::vector<Blaze::PathTag>> m_tags;
    std::vector<std::vector<Blaze::FloatPoint>> m_points;
    std::vector<Blaze::Geometry> m_geometries;
    Blaze::DestinationImage<Blaze::TileDescriptor_16x8> m_image;
    Blaze::VectorImage m_vector_image;
    Blaze::Matrix m_coordinate_system;

    Viewport m_viewport;
  private:
    static Renderer* s_instance;
  };

}
