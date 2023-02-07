#pragma once

#include "../../utils/uuid.h"

class Entity;

class Component {
public:
  const UUID id;
  Entity* parent;
public:
  Component(Entity* entity): parent(entity) {};
  Component(const Component&) = default;
  Component(Component&&) = default;

  ~Component() = default;
};
