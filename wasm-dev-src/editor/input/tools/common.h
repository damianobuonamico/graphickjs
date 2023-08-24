#pragma once

#include "../../../renderer/geometry/internal.h"

namespace Graphick::Editor::Input {

  class SelectionRect {
  public:
    SelectionRect(bool dashed = false);
    SelectionRect(const SelectionRect&) = default;
    SelectionRect(SelectionRect&&) = default;

    ~SelectionRect() = default;

    inline bool active() const { return m_active; }
    inline vec2 position() const { return m_position; }
    inline Renderer::Geometry::Internal::PathInternal path() const { return m_path; }

    rect bounding_rect() const;

    void set(const vec2 position);
    void size(const vec2 size);
    void reset();
  private:
    bool m_dashed = false;
    bool m_active = false;

    vec2 m_position;
    vec2 m_anchor_position;
    Renderer::Geometry::Internal::PathInternal m_path;
  };

}
