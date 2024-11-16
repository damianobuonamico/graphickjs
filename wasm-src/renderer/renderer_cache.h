/**
 * @file renderer/renderer_cache.h
 * @brief This file contains the definition of the RendererCache class.
 *
 * @todo clear elements from cache after a few unused frames
 */

#pragma once

#include "../math/rect.h"

#include "../utils/defines.h"
#include "../utils/uuid.h"

// TEMP
#include "../utils/console.h"

#include "drawable.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace graphick::renderer {

/**
 * @brief The RendererCache class is used to store cached data.
 *
 * It is designed to be validated exclusively by the History class.
 */
class RendererCache {
 public:
  /**
   * @brief Clears the cache.
   */
  void clear();

  /**
   * @brief Clears the cache of a specific element.
   *
   * @param id The id of the element to clear.
   */
  inline void clear(uuid id)
  {
    m_bounding_rects.erase(id);
    m_drawables.erase(id);
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
  inline const std::vector<rect>& get_invalid_rects() const
  {
    return m_invalid_rects;
  }

  inline const drect& get_bounding_rect(uuid id)
  {
    return m_bounding_rects.at(id);
  }

  inline const drect& get_bounding_rect(uuid id, const std::function<drect()>&& callback_fn)
  {
    if (!has_bounding_rect(id)) {
      m_bounding_rects.insert({id, callback_fn()});
    }

    return m_bounding_rects.at(id);
  }

  inline void set_bounding_rect(uuid id, const drect& bounding_rect)
  {
    m_bounding_rects.insert({id, bounding_rect});
  }

  inline bool has_bounding_rect(uuid id) const
  {
    return m_bounding_rects.find(id) != m_bounding_rects.end();
  }

  inline const Drawable& get_drawable(uuid id)
  {
    return m_drawables.at(id);
  }

  inline void set_drawable(uuid id, Drawable&& drawable)
  {
    m_drawables.insert({id, drawable});
  }

  inline bool has_drawable(uuid id) const
  {
    return m_drawables.find(id) != m_drawables.end();
  }

 private:
  std::unordered_map<uuid, drect> m_bounding_rects;  // The bounding rectangles of the paths.
  std::unordered_map<uuid, Drawable> m_drawables;    // The drawables.

  std::vector<bool> m_grid;  // When an action is performed, some grid cells are invalidated.
  std::vector<rect> m_invalid_rects;  // The invalid rectangles.

  ivec2 m_subdivisions;               // The number of subdivisions in the grid.
  rect m_grid_rect;                   // The portion of the screen that is cached.
};

}  // namespace graphick::renderer
