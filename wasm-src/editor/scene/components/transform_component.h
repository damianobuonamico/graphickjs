#pragma once

#include "../component.h"
#include "../../../values/vec2_value.h"
#include "../../../math/box.h"

class TransformComponent: public Component {
public:
  TransformComponent(Entity* entity): Component(entity) {};
  TransformComponent(const TransformComponent&) = default;
  TransformComponent(TransformComponent&&) = default;

  ~TransformComponent() = default;

  inline Vec2Value& position() { return m_position; };

  virtual Box bounding_box() const;

  virtual void translate(const vec2& amount, bool apply);
  virtual void translate_to(const vec2& value, bool apply);

  inline virtual void apply() { m_position.apply(); };

  virtual vec2 transform(const vec2& point) const;
protected:
  Vec2Value m_position;
};

class CircleTransformComponent: public TransformComponent {
public:
  CircleTransformComponent(Entity* entity, float* radius)
    : TransformComponent(entity), m_radius(radius) {};
  CircleTransformComponent(const CircleTransformComponent&) = default;
  CircleTransformComponent(CircleTransformComponent&&) = default;

  ~CircleTransformComponent() = default;

  virtual Box bounding_box() const override;
private:
  float* m_radius;
};

class VertexTransformComponent: public TransformComponent {
public:
  VertexTransformComponent(Entity* entity)
    : TransformComponent(entity) {};
  VertexTransformComponent(const VertexTransformComponent&) = default;
  VertexTransformComponent(VertexTransformComponent&&) = default;

  ~VertexTransformComponent() = default;

  Vec2Value* left();
  Vec2Value* right();

  virtual Box bounding_box() const override;

  void translate_left(const vec2& amount, bool mirror, bool apply);
  void translate_right(const vec2& amount, bool mirror, bool apply);
  void translate_left_to(const vec2& value, bool mirror, bool apply);
  void translate_right_to(const vec2& value, bool mirror, bool apply);

  virtual void apply() override;
};
