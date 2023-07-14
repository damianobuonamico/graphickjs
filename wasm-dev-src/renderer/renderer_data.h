#pragma once

#include "../math/ivec2.h"
#include "../math/vec2.h"
#include "../math/vec4.h"

#include "../utils/uuid.h"

namespace Graphick::Renderer {

  struct Viewport {
    ivec2 size;
    double dpr;

    vec2 position;
    double zoom;

    vec4 background;
  };

  struct BatchedLinesData {
    uuid instance_buffer_id = 0;

    uint32_t instances = 0;

    vec4* instance_buffer = nullptr;
    vec4* instance_buffer_ptr = nullptr;

    uint32_t max_instance_buffer_size = (uint32_t)std::pow(2, 20);
    uint32_t max_instance_count = max_instance_buffer_size / sizeof(vec4);

    BatchedLinesData() {
      instance_buffer = new vec4[max_instance_count];
      instance_buffer_ptr = instance_buffer;
    }
    BatchedLinesData(BatchedLinesData const&) = delete;
    BatchedLinesData& operator=(BatchedLinesData const&) = delete;

    ~BatchedLinesData() {
      delete[] instance_buffer;
    }
  };

}
