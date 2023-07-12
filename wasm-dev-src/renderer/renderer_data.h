#pragma once

#include "../math/ivec2.h"
#include "../math/vec2.h"
#include "../math/vec4.h"

namespace Graphick::Renderer {

  struct Viewport {
    ivec2 size;
    double dpr;

    vec2 position;
    double zoom;

    vec4 background;
  };

}
