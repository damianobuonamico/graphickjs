#pragma once

#include "../math/ivec2.h"
#include "../math/vec2.h"
#include "../math/vec4.h"

#include "../utils/uuid.h"

#include <vector>
#include <string>

#define TILE_SIZE 64
#define MASKS_TEXTURE_SIZE (TILE_SIZE * 32)
#define MASKS_PER_BATCH ((MASKS_TEXTURE_SIZE / TILE_SIZE) * (MASKS_TEXTURE_SIZE / TILE_SIZE))

namespace Graphick::Renderer {

  struct Viewport {
    ivec2 size;
    float dpr;

    vec2 position;
    float zoom;

    vec4 background;
  };

  struct InstancedLinesData {
    uuid instance_buffer_id = 0;
    uuid vertex_buffer_id = 0;

    uint32_t instances = 0;

    vec4* instance_buffer = nullptr;
    vec4* instance_buffer_ptr = nullptr;

    uint32_t max_instance_buffer_size = (uint32_t)std::pow(2, 20);
    uint32_t max_instance_count = max_instance_buffer_size / sizeof(vec4);

    InstancedLinesData() {
      instance_buffer = new vec4[max_instance_count];
      instance_buffer_ptr = instance_buffer;
    }
    InstancedLinesData(InstancedLinesData const&) = delete;
    InstancedLinesData& operator=(InstancedLinesData const&) = delete;

    ~InstancedLinesData() {
      delete[] instance_buffer;
    }
  };

  struct InstancedMeshData {
    std::string name;

    uuid instance_buffer_id = 0;
    uuid vertex_buffer_id = 0;
    uuid index_buffer_id = 0;

    std::vector<vec2> instances;

    uint32_t buffer_size = 0;

    InstancedMeshData(std::string name) : name(name) {}
  };

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

}
