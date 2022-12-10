#include "renderer/renderer.h"
#include <stdio.h>
#include <cmath>
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Renderer) {
  function("_init", Renderer::init);
}