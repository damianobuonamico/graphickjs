#pragma once

#include "../../renderer/renderer.h"
#include "../../utils/uuid.h"
#include "../../math/box.h"
#include "components/transform_component.h"
#include "components/selection_component.h"

class Entity {
public:
  const UUID id;
  Entity* parent = nullptr;
public:
  enum Category {
    CategoryNone = 0,
    CategorySelectable = 1 << 0,
    CategorySelectableChildren = 1 << 1,
  };
public:
  Entity(Entity* parent): parent(parent), m_category(CategoryNone) {}
  Entity(int category = CategoryNone): m_category(category) {}
  Entity(const Entity&) = default;
  Entity(Entity&&) = default;

  ~Entity() = default;

  inline bool is_in_category(Category category) const { return m_category & category; }

  virtual TransformComponent& transform() = 0;
  virtual const TransformComponent& transform() const = 0;

  virtual SelectionComponent* selection() { return nullptr; }
  virtual const SelectionComponent* selection() const { return nullptr; }

  virtual void tessellate_outline(const vec4& color, float zoom, Geometry& geo) const {}
  virtual void render(float zoom) const {};

  virtual Entity* entity_at(const vec2& position, bool lower_level, float threshold) { return nullptr; }
  virtual void entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) {}

  virtual Entity* duplicate() const { return nullptr; };
protected:
  const int m_category;
};
