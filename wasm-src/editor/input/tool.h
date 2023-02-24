#pragma once

#include "../../math/vec4.h"
#include "../../renderer/geometry/geometry.h"

class Tool {
public:
  enum class ToolType {
    Pan = 0,
    Zoom,
    Select,
    DirectSelect,
    Pencil,
    None
  };

  enum Category {
    CategoryNone = 0,
    CategoryDirect = 1 << 0,
    CategoryImmediate = 1 << 1,
  };
public:
  Tool() = delete;
  Tool(const Tool&) = delete;
  Tool(Tool&&) = delete;

  ~Tool() = default;

  inline ToolType type() const { return m_type; }
  inline int category() const { return m_category; }
  inline bool is_in_category(Category category) const { return m_category & category; }

  virtual void on_pointer_down() {}
  virtual void on_pointer_move() {}
  virtual void on_pointer_up(bool abort = false) {}

  virtual void on_pointer_hover() {}

  virtual void tessellate_overlays_outline(const vec4& color, float zoom, Geometry& geo) const {}
  virtual void render_overlays(float zoom) const {}
protected:
  Tool(ToolType type, int category): m_type(type), m_category(category) {}
protected:
  const ToolType m_type;
  const int m_category;
};
