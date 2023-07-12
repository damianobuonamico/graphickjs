#pragma once

#if 1 
#include "renderer_data.h"
#include "gpu/shaders.h"

// TEMP
#include "../editor/scene/viewport.h"

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
    static void begin_frame(const Editor::Viewport& viewport) {};
    static void end_frame() {};

    static void draw(const Geometry::Path& path) {};
    // TODO: Batch outline draw calls
    static void draw_outline(const Geometry::Path& path) {};
  private:
    Renderer() = default;
    ~Renderer() = default;

    static inline Renderer* get() { return s_instance; }

    void update_image_data(const Viewport& viewport);
    void render_frame_backend(const Viewport& viewport);

    // TODO: Move to viewport
    Blaze::Matrix get_matrix(const Viewport& viewport);
  private:
    GPU::Programs m_programs;

#ifdef EMSCRIPTEN
    int m_ctx;
#endif

    uuid m_quad_vertex_positions_buffer_id = 0;
    uuid m_quad_vertex_indices_buffer_id = 0;
    uuid m_frame_texture_id = 0;

    Blaze::DestinationImage<Blaze::TileDescriptor_16x8> m_image;
    Blaze::VectorImage m_vector_image;
    Blaze::Matrix m_coordinate_system;

    // TODO: Move to viewport
    vec2 m_translation;
    double m_scale = 1;
  private:
    static Renderer* s_instance;
  };

}
#else

#include "tiler.h"
#include "gpu/shaders.h"

#include "../math/ivec2.h"
#include "../math/vec2.h"
#include "../math/mat4.h"

#include "../editor/scene/viewport.h"

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

    static void begin_frame(const Editor::Viewport& viewport);
    static void end_frame();

    static void draw(const Geometry::Path& path);
    // TODO: Batch outline draw calls
    static void draw_outline(const Geometry::Path& path);
  private:
    Renderer() = default;
    ~Renderer() = default;

    static inline Renderer* get() { return s_instance; }

    void draw_opaque_tiles();
    void draw_masked_tiles();
  private:
    GPU::Programs m_programs;

    mat4 m_projection;
    mat4 m_translation;
    mat4 m_tiles_projection;
    mat4 m_tiles_translation;

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
#endif
