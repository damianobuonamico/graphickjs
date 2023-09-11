#include "common.h"

#include "../../../utils/console.h"

namespace Graphick::Editor::Input {

  SelectionRect::SelectionRect(bool dashed) :
    m_position({ 0.0f, 0.0f }), m_anchor_position({ 0.0f, 0.0f }),
    m_dashed(dashed)
  {
    m_path.move_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.line_to({ 0.0f, 0.0f });
    m_path.close();
  }

  Math::rect SelectionRect::bounding_rect() const {
    return m_path.bounding_rect() + m_position;
  }

  void SelectionRect::set(const vec2 position) {
    m_anchor_position = m_position = position;
    m_active = true;

    size({ 0.0f, 0.0f });
  }

  void SelectionRect::size(const vec2 size) {
    const auto& segments = m_path.segments();

    if (segments.size() < 4) {
      console::error("SelectionRect has less than 4 sides!");
      return;
    }

    vec2 new_size = size;

    m_position = m_anchor_position;

    if (size.x < 0) {
      m_position.x = m_anchor_position.x + size.x;
      new_size.x = -size.x;
    }
    if (size.y < 0) {
      m_position.y = m_anchor_position.y + size.y;
      new_size.y = -size.y;
    }

    *segments[0].p3_ptr().lock() = { new_size.x, 0.0f };
    *segments[1].p3_ptr().lock() = new_size;
    *segments[2].p3_ptr().lock() = { 0.0f, new_size.y };
  }

  void SelectionRect::reset() {
    m_position = m_anchor_position;
    m_active = false;

    size({ 0.0f, 0.0f });
  }

}
