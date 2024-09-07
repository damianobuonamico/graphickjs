/**
 * @file editor/scene/cache.h
 * @brief This file contains the definition of the Cache class.
 */

#pragma once

#include "../../math/rect.h"

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
     * @brief Sets the portion of the screen that is cached.
     *
     * @param grid_rect The visible rectangle.
     */
    void set_grid_rect(const rect grid_rect, const ivec2 subdivisions);

    /**
     * @brief Invalidates a rectangle in the cache.
     *
     * @param invalidated_rect The rectangle to invalidate.
     */
    void invalidate_rect(const rect invalidated_rect);

    /**
     * @brief Gets the valid grid cells in the cache.
     *
     * @return The valid rectangles.
     */
    std::vector<rect> get_invalid_rects() const;
  private:
    std::vector<bool> m_grid;    /* When an action is performed, some grid cells are invalidated. */

    ivec2 m_subdivisions;        /* The number of subdivisions in the grid. */
    rect m_grid_rect;            /* The portion of the screen that is cached. */
  };

}