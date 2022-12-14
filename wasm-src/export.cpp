#include "common.h"
#include "renderer/renderer.h"

#include <emscripten/bind.h>

using namespace emscripten;

extern "C" {
  void EMSCRIPTEN_KEEPALIVE begin_frame(const float* position, const float zoom) {
    Renderer::begin_frame(position, zoom);
  }

  void EMSCRIPTEN_KEEPALIVE draw(const float* vertices, const float vertices_length, const float* indices, const float indices_length) {
    Geometry geometry;

    for (int i = 0; i < vertices_length; i += 2) {
      geometry.vertices.push_back({ {vertices[i], vertices[i + 1]} });
    }

    for (int i = 0; i < indices_length; ++i) {
      geometry.indices.push_back(indices[i]);
    }

    Renderer::draw(geometry);
  }
}

EMSCRIPTEN_BINDINGS(Renderer) {
  function("_init", Renderer::init);
  function("_resize", Renderer::resize);
  function("_end_frame", Renderer::end_frame);
}