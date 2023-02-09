#pragma once

#include "handle_entity.h"

#include <optional>

class VertexTransformComponent;

class VertexEntity: public Entity {
public:
  using OptionalHandle = std::optional<HandleEntity>;
public:
  VertexEntity(const vec2& position)
    : m_transform(VertexTransformComponent{ this }),
    m_position(position, HandleEntity::Type::Square),
    m_left(std::nullopt), m_right(std::nullopt) {
    console::log("VertexEntity created");
  };
  VertexEntity(const vec2& position, const vec2& handle, bool is_left)
    : m_transform(VertexTransformComponent{ this }),
    m_position(position, HandleEntity::Type::Square),
    m_left(is_left ? OptionalHandle{ {handle, HandleEntity::Type::Circle} } : std::nullopt),
    m_right(is_left ? std::nullopt : OptionalHandle{ {handle, HandleEntity::Type::Circle} }) {}
  VertexEntity(const vec2& position, const vec2& left, const vec2& right)
    : m_transform(VertexTransformComponent{ this }),
    m_position(position, HandleEntity::Type::Square),
    m_left(OptionalHandle{ {left, HandleEntity::Type::Circle} }),
    m_right(OptionalHandle{ {right, HandleEntity::Type::Circle} }) {}
  VertexEntity(const VertexEntity&) = default;
  VertexEntity(VertexEntity&&) = default;

  ~VertexEntity() {
    console::log("VertexEntity destroyed");
  }

  inline virtual VertexTransformComponent& transform() override { return m_transform; }
  inline virtual const VertexTransformComponent& transform() const override { return m_transform; }

  inline HandleEntity* position() { return &m_position; };
  inline HandleEntity* left() { return m_left.has_value() ? &m_left.value() : nullptr; };
  inline HandleEntity* right() { return m_right.has_value() ? &m_right.value() : nullptr; };

  virtual void render(float zoom) const override;

  virtual Entity* entity_at(const vec2& position, bool lower_level, float threshold) override;
  virtual void entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) override;
private:
  HandleEntity m_position;
  OptionalHandle m_left;
  OptionalHandle m_right;

  VertexTransformComponent m_transform;
};
