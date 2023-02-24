#pragma once

#include "../component.h"
#include "../../../values/vec2_value.h"
#include "../../../math/box.h"

class TransformComponent: public Component {
public:
  TransformComponent(Entity* entity): Component(entity) {};
  TransformComponent(Entity* entity, const vec2& position)
    : Component(entity), m_position({ position }) {};
  TransformComponent(const TransformComponent&) = default;
  TransformComponent(TransformComponent&&) = default;

  ~TransformComponent() = default;

  // TODO: fix const
  virtual inline Vec2Value& position() { return m_position; };
  virtual inline const Vec2Value& position() const { return m_position; };

  virtual Box bounding_box() const;
  virtual vec2 dimensions() const;
  virtual vec2 center() const;

  virtual void translate(const vec2& amount, bool apply = false);
  virtual void translate_to(const vec2& value, bool apply = false);

  inline virtual void apply() { m_position.apply(); };

  virtual vec2 transform(const vec2& point) const;
protected:
  Vec2Value m_position;
};

class CircleTransformComponent: public TransformComponent {
public:
  CircleTransformComponent(Entity* entity, const vec2& position, float radius)
    : TransformComponent(entity, position), m_radius(radius) {};
  CircleTransformComponent(const CircleTransformComponent&) = default;
  CircleTransformComponent(CircleTransformComponent&&) = default;

  ~CircleTransformComponent() = default;

  inline float radius() const { return m_radius; };

  virtual Box bounding_box() const override;
private:
  float m_radius;
};

class RectTransformComponent: public TransformComponent {
public:
  RectTransformComponent(Entity* entity, const vec2& position, const vec2& size)
    : TransformComponent(entity, position), m_size(size) {};
  RectTransformComponent(const RectTransformComponent&) = default;
  RectTransformComponent(RectTransformComponent&&) = default;

  ~RectTransformComponent() = default;

  inline Vec2Value& size() { return m_size; };
  inline const Vec2Value& size() const { return m_size; };

  virtual Box bounding_box() const override;
private:
  Vec2Value m_size;
};

class VertexTransformComponent: public TransformComponent {
public:
  VertexTransformComponent(Entity* entity)
    : TransformComponent(entity) {};
  VertexTransformComponent(const VertexTransformComponent&) = default;
  VertexTransformComponent(VertexTransformComponent&&) = default;

  ~VertexTransformComponent() = default;

  virtual Vec2Value& position() override;
  Vec2Value* left();
  Vec2Value* right();

  virtual Box bounding_box() const override;

  virtual void translate(const vec2& amount, bool apply = false) override;
  virtual void translate_to(const vec2& value, bool apply = false) override;
  void translate_left(const vec2& amount, bool mirror = false, bool apply = false);
  void translate_right(const vec2& amount, bool mirror = false, bool apply = false);
  void translate_left_to(const vec2& value, bool mirror = false, bool apply = false);
  void translate_right_to(const vec2& value, bool mirror = false, bool apply = false);

  virtual void apply() override;
};

class ElementTransformComponent: public TransformComponent {
public:
  ElementTransformComponent(Entity* entity)
    : TransformComponent(entity) {};
  ElementTransformComponent(Entity* entity, const vec2& position)
    : TransformComponent(entity, position) {};
  ElementTransformComponent(const ElementTransformComponent&) = default;
  ElementTransformComponent(ElementTransformComponent&&) = default;

  ~ElementTransformComponent() = default;

  virtual Box bounding_box() const override;
  Box large_bounding_box() const;

  virtual void apply() override;
private:
  vec2 m_origin{};
};
