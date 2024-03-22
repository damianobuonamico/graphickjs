/**
 * @file renderer.cpp
 * @brief The file contains the implementation of the main Graphick renderer.
 */

#pragma once

#include "renderer_new.h"

#include "gpu/allocator.h"
#include "gpu/device.h"

#include "../utils/assert.h"

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace Graphick::renderer {

  // TEMP
  namespace GPU = Graphick::Renderer::GPU;

  /* -- Static -- */

  /**
   * @brief Generates an orthographic projection matrix.
   *
   * @param size The size of the viewport.
   * @param zoom The zoom level.
   * @return The orthographic projection matrix p.
   */
  static dmat4 orthographic_projection(const vec2 size, double zoom) {
    const dvec2 dsize = dvec2(size);

    const double factor = 0.5f / zoom;
    const double half_width = -dsize.x * factor;
    const double half_height = dsize.y * factor;

    const double right = -half_width;
    const double left = half_width;
    const double top = -half_height;
    const double bottom = half_height;

    return dmat4{
      2.0 / (right - left), 0.0, 0.0, 0.0,
      0.0, 2.0 / (top - bottom), 0.0, 0.0,
      0.0, 0.0, -1.0, 0.0,
      -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0, 1.0
    };
  }

  /**
   * @brief Generates an orthographic translation matrix.
   *
   * @param size The size of the viewport.
   * @param position The position of the camera.
   * @param zoom The zoom level.
   * @return The orthographic translation matrix v.
   */
  static dmat4 orthographic_translation(const vec2 size, const vec2 position, const double zoom) {
    const dvec2 dsize = dvec2(size);
    const dvec2 dposition = dvec2(position);

    return dmat4{
      1.0f, 0.0f, 0.0f, 0.5f * (-dsize.x / zoom + 2 * dposition.x),
      0.0f, 1.0f, 0.0f, 0.5f * (-dsize.y / zoom + 2 * dposition.y),
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  /**
   * @brief Generates a model matrix from a 2x3 transformation matrix.
   *
   * @param transform The 2x3 transformation matrix.
   * @return The model matrix.
   */
  static dmat4 model_matrix(const mat2x3& transform) {
    return dmat4{
      static_cast<double>(transform[0][0]), static_cast<double>(transform[0][1]), static_cast<double>(transform[0][2]), 0.0,
      static_cast<double>(transform[1][0]), static_cast<double>(transform[1][1]), static_cast<double>(transform[1][2]), 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0
    };
  }

  /**
   * @brief Flushes the instanced data to the GPU.
   *
   * Here the GPU draw calls are actually issued.
   *
   * @param data The instanced data to flush.
   */
  template<typename T>
  static void flush(InstancedData<T>& data) {
    if (data.instances.empty()) {
      return;
    }

    const GPU::Buffer instance_buffer = GPU::Memory::Allocator::create_buffer(data.instances.data(), data.instances.size() * sizeof(T), GPU::BufferUsage::DynamicDraw);
  }

  /* -- Static member initialization -- */

  Renderer* Renderer::s_instance = nullptr;

  /* -- Renderer -- */

  void Renderer::init() {
    GK_ASSERT(s_instance == nullptr, "Renderer already initialized, call shutdown() before reinitializing!");

#ifdef EMSCRIPTEN
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);

    /* https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/WebGL_best_practices#avoid_alphafalse_which_can_be_expensive */
    attr.alpha = true;
    attr.premultipliedAlpha = false;
    attr.majorVersion = 2;
    attr.antialias = false;
    attr.stencil = false;
    attr.depth = true;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);

    GPU::Device::init(GPU::DeviceVersion::GLES3, 0);
#else
    GPU::Device::init(GPU::DeviceVersion::GL3, 0);
#endif
    GPU::Memory::Allocator::init();

    s_instance = new Renderer();
  }

  void Renderer::shutdown() {
    GK_ASSERT(s_instance != nullptr, "Renderer not initialized, call init() before shutting down!");

    delete s_instance;
    s_instance = nullptr;

    GPU::Memory::Allocator::shutdown();
    GPU::Device::shutdown();

#ifdef EMSCRIPTEN
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
    emscripten_webgl_destroy_context(ctx);
#endif
  }

  void Renderer::begin_frame(const Viewport& viewport) {
    const dmat4 view_matrix = orthographic_translation(viewport.size, viewport.position, viewport.zoom);
    const dmat4 projection_matrix = orthographic_projection(viewport.size, viewport.zoom);

    get()->m_viewport = viewport;
    get()->m_vp_matrix = projection_matrix * view_matrix;

    get()->m_path_instances.clear();
    get()->m_transforms.clear();

    GPU::Device::begin_commands();
    GPU::Device::set_viewport(viewport.size);
    GPU::Device::clear({ viewport.background, 1.0f, std::nullopt });
  }

  void Renderer::end_frame() {
    get()->flush(get()->m_path_instances);

    GPU::Memory::Allocator::purge_if_needed();
    GPU::Device::end_commands();
  }

  void Renderer::draw(const geometry::QuadraticPath& path, const Stroke& stroke, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
    const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();

    draw(path, fill, transform, &bounds);
    draw(path, stroke, transform, &bounds);
  }

  void Renderer::draw(const geometry::QuadraticPath& path, const Stroke& stroke, const mat2x3& transform, const rect* bounding_rect) {
    if (path.empty()) {
      return;
    }

    // Will call fill once stroke path is calcualted
  }

  void Renderer::draw(const geometry::QuadraticPath& path, const Fill& fill, const mat2x3& transform, const rect* bounding_rect) {
    if (path.empty()) {
      return;
    }

    const rect bounds = bounding_rect ? *bounding_rect : path.approx_bounding_rect();
    const mat4 mvp = mat4(get()->m_vp_matrix * model_matrix(transform));

    get()->m_transforms.push_back(mvp);
    get()->m_path_instances.instances.push_back({ vec2{ 100.0f, 100.0f }, static_cast<uint32_t>(get()->m_transforms.size() - 1) });
  }

  void Renderer::draw_outline(const geometry::QuadraticPath& path, const mat2x3& transform, bool draw_vertices, const std::unordered_set<size_t>* selected_vertices, const Stroke* stroke, const rect* bounding_rect) {
  }

  Renderer::Renderer() :
    m_path_instances(GK_LARGE_BUFFER_SIZE) {}

// void Renderer::flush(InstancedData<T>& data) {

// }

}
