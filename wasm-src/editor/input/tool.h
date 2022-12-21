#pragma once

class Tool {
public:
  enum class ToolType {
    Pan = 0,
    Zoom = 1,
    Pencil = 2,
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

  virtual void on_pointer_down() {};
  virtual void on_pointer_move() {};
  virtual void on_pointer_up() {};

  virtual void on_pointer_hover() {};
protected:
  Tool(ToolType type, int category): m_type(type), m_category(category) {}
protected:
  const ToolType m_type;
  const int m_category;
};
