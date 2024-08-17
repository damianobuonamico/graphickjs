/**
 * @file editor/scene/cache.h
 * @brief This file contains the definition of the Cache class.
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/vec4.h"

#include <vector>

namespace graphick::editor {

  /**
   * @brief The Cache class is used to store cached data.
   *
   * It is designed to be validated exclusively by the History class.
   * The main object stored is the scene grid.
   *
   * @class Cache
   */
  class Cache {
  public:
    struct GridCell {
      bool valid;

      vec2 position;
      std::vector<uvec4> pixels;

      
    };
  private:
    std::vector<GridCell> grid;    /* The grid of the scene, made of 256x256 (pixels) cells. */
  };

}