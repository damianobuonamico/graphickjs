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
   *
   * @class Cache
   */
  class Cache {
  public:
    /**
     * @brief Clears the cache.
     */
    void clear();

    /**
     * @brief Returns the size of the rendered scene cache.
     * @return The size of the cache.
     */
    inline ivec2 size() const {
      return m_size;
    }

    /**
     * @brief Returns the pixels of the rendered scene cache.
     * @return The pixels of the cache.
     */
    inline uvec4* pixels() {
      return m_pixels.data();
    }

    /**
     * @brief Resizes the rendered scene cache.
     * @param size The new size of the cache.
     */
    inline void resize(const ivec2 size) {
      m_pixels.resize(size.x * size.y);
    }
  private:
    ivec2 m_size;                   /* The size of the rendered scene cache. */

    std::vector<uvec4> m_pixels;    /* The cached rendered scene. */
  };

}