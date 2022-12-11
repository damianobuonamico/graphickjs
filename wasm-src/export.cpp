#include "common.h"
#include "renderer/renderer.h"

#include <emscripten/bind.h>

using namespace emscripten;

extern "C" {
  void EMSCRIPTEN_KEEPALIVE begin_frame(const float* position, const float zoom) {
    Renderer::begin_frame(position, zoom);
  }
}

EMSCRIPTEN_BINDINGS(Renderer) {
  function("_init", Renderer::init);
  function("_resize", Renderer::resize);
  function("_end_frame", Renderer::end_frame);
}