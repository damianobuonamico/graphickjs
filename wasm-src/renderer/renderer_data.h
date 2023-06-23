#pragma once

#include "gpu/gpu_data.h"

#define SEGMENTS_TEXTURE_SIZE 2048

namespace Graphick::Render {

  struct Viewport {
    ivec2 size;
    float dpr;
    vec2 position;
    float zoom;
  };

  struct Tile {
    // TODO: Replace with a pointer to the color texture
    vec4 color;
    int32_t index;
    int32_t mask_index;
  };

  struct Span {
    // TODO: Replace with a pointer to the color texture
    vec4 color;
    int32_t index;
    int16_t width;
  };

}
