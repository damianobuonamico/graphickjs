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
  HandleEntity(const vec2& position, Type type = Type::Circle)
    : m_transform(CircleTransformComponent{ this, position, &m_radius }), type(type) {
    console::log("HandleEntity created");
  };
  HandleEntity(const HandleEntity&) = default;
  HandleEntity(HandleEntity&&) = default;

  ~HandleEntity() {
    console::log("HandleEntity destroyed");
  }

  inline virtual CircleTransformComponent& transform() override { return m_transform; }

  virtual void render(float zoom) override;

  virtual Entity* entity_at(const vec2& position, bool lower_level = false, float threshold = 0.0f) override;
private:
  float m_radius = 5.0f;

  CircleTransformComponent m_transform;
};
