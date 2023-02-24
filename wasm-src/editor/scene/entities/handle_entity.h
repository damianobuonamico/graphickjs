#pragma once

#include "../entity.h"
#include "../../../math/math.h"

class HandleEntity: public Entity {
public:
  enum class Type {
    Square = 0,
    Circle,
  };

  Type type;
public:
  HandleEntity(const vec2& position, Type type = Type::Circle, Entity* parent = nullptr)
    : Entity(parent), m_transform(CircleTransformComponent{ this, position, 5.0f }), type(type) {
    console::log("HandleEntity created");
  };
  HandleEntity(const HandleEntity&) = default;
  HandleEntity(HandleEntity&&) = default;

  ~HandleEntity() {
    console::log("HandleEntity destroyed");
  }

  inline virtual CircleTransformComponent& transform() override { return m_transform; }
  inline const CircleTransformComponent& transform() const override { return m_transform; }

  virtual void render(float zoom) const override;

  virtual Entity* entity_at(const vec2& position, bool lower_level = false, float threshold = 0.0f) override;
private:
  CircleTransformComponent m_transform;
};
