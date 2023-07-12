#pragma once

#include "../math/ivec2.h"
#include "../math/vec2.h"
#include "../math/vec4.h"

#define MASKS_TEXTURE_SIZE (16 * 100)

namespace Graphick::Renderer {

  struct OpaqueTile {
    // TODO: Replace with a pointer to the color texture
    vec4 color;
    int32_t index;
  };

  struct MaskedTile {
    // TODO: Replace with a pointer to the color texture
    vec4 color;
    int32_t index;
    int32_t mask_index;
  };

  struct Viewport {
    ivec2 size;
    float dpr;

    vec2 position;
    float zoom;

    vec4 background;
  };

}
