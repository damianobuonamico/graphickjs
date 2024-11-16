/**
 * @file editor/scene/cache.h
 * @brief This file contains the definition of the Cache class.
 */

#pragma once

#include "../../math/rect.h"

#include "../../renderer/renderer_cache.h"

#include <vector>

namespace graphick::editor {

/**
 * @brief The Cache class is used to store cached data.
 *
 * It is designed to be validated exclusively by the History class.
 */
class Cache {
 public:
  renderer::RendererCache renderer_cache;  // The renderer cache.
 public:
  /**
   * @brief Clears the cache.
   */
  void clear();

  /**
   * @brief Clears the cache of the given entity.
   *
   * @param entity_id The id of the entity to clear.
   */
  inline void clear(const uuid entity_id)
  {
    renderer_cache.clear(entity_id);
  }

  /**
   * @brief Sets the portion of the screen that is cached.
   *
   * This method should be called at the end of each frame.
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
  const std::vector<rect>& get_invalid_rects() const
  {
    return m_invalid_rects;
  }

 private:
  std::vector<bool> m_grid;  // When an action is performed, some grid cells are invalidated.
  std::vector<rect> m_invalid_rects;  // The invalid rectangles.

  ivec2 m_subdivisions;               // The number of subdivisions in the grid.
  rect m_grid_rect;                   // The portion of the screen that is cached.
};

}  // namespace graphick::editor
