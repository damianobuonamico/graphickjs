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

  struct Line {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
  };

  struct Mask {
    Line line_segment;
    int32_t index;
  };

  struct Tile {
    vec4 color;
    int32_t index;
    int32_t mask_index;
  };

}
