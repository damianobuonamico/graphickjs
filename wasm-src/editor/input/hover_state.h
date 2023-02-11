#pragma once

#include "../scene/entity.h"

class HoverState {
public:
  HoverState() = default;

  HoverState(const HoverState&) = delete;
  HoverState(HoverState&&) = delete;

  ~HoverState() = default;

  inline Entity* entity() const { return m_entity; }
  Entity* element() const;
private:
  inline void set(Entity* entity) { m_entity = entity; }
private:
  Entity* m_entity = nullptr;
private:
  friend class InputManager;
};
