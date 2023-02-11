#pragma once

#include "../component.h"

class SelectionComponent: public Component {
public:
  SelectionComponent(Entity* entity): Component(entity) {};
  SelectionComponent(const SelectionComponent&) = default;
  SelectionComponent(SelectionComponent&&) = default;

  ~SelectionComponent() = default;

  inline size_t size() const { return 0; }

  void clear() {};
  void all() {};
  void sync() {};
};
