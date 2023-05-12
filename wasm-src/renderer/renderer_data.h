#pragma once

#include "gpu/gpu_data.h"

namespace Graphick::Render {

  struct Viewport {
    ivec2 size;
    float dpr;
    vec2 position;
    float zoom;
  };

  struct Fill {
    vec4 color;
    int32_t index;
  };

}
