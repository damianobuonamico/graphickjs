/**
 * @file cache.cpp
 * @brief This file contains the implementation of the cache class.
 */

#include "cache.h"

#include "../../math/vector.h"

namespace graphick::editor {

  void Cache::clear() {
    for (int i = 0; i < m_grid.size(); i++) {
      m_grid[i] = false;
    }
  }

  void Cache::set_grid_rect(const rect grid_rect, const ivec2 subdivisions) {
    m_subdivisions = subdivisions;
    m_grid_rect = grid_rect;

    m_grid.resize(subdivisions.x * subdivisions.y);

    for (int i = 0; i < m_grid.size(); i++) {
      m_grid[i] = true;
    }

    m_invalid_rects.clear();
  }

  void Cache::invalidate_rect(const rect invalidated_rect) {
    const vec2 cell_size = m_grid_rect.size() / vec2(m_subdivisions);
    const rect translated_rect = invalidated_rect - m_grid_rect.min;
    const ivec2 start_coords = math::max(ivec2::zero(), ivec2(math::floor(translated_rect.min / cell_size)));
    const ivec2 end_coords = math::min(m_subdivisions, ivec2(math::ceil(translated_rect.max / cell_size)));

    for (int y = start_coords.y; y < end_coords.y; y++) {
      for (int x = start_coords.x; x < end_coords.x; x++) {
        if (m_grid[y * m_subdivisions.x + x] == true) {
          m_grid[y * m_subdivisions.x + x] = false;

          m_invalid_rects.push_back(rect{
            m_grid_rect.min + vec2(x, y) * cell_size,
            m_grid_rect.min + vec2(x + 1, y + 1) * cell_size
            });
        }
      }
    }
  }

}