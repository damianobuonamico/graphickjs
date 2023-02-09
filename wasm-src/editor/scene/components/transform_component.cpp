#include "transform_component.h"

#include "../../../math/vector.h"
#include "../entities/element_entity.h"

/* -- TransformComponent -- */

Box TransformComponent::bounding_box() const {
  return { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
}

vec2 TransformComponent::size() const {
  Box box = bounding_box();
  return box.max - box.min;
}

vec2 TransformComponent::center() const {
  Box box = bounding_box();
  return midpoint(box.max, box.min);
}

void TransformComponent::translate(const vec2& amount, bool apply = false) {
  if (apply) {
    m_position.add(amount);
  } else {
    m_position.add_delta(amount);
  }
}

void TransformComponent::translate_to(const vec2& value, bool apply = false) {
  if (apply) {
    m_position.set(value);
  } else {
    m_position.move_to(value);
  }
}

vec2 TransformComponent::transform(const vec2& point) const {
  return point + m_position.get();
}

/* -- CircleTransformComponent -- */

Box CircleTransformComponent::bounding_box() const {
  const vec2 position = m_position.get();
  return { position - *m_radius, position + *m_radius };
}

/* -- VertexTransformComponent -- */

Vec2Value& VertexTransformComponent::position() {
  return static_cast<VertexEntity*>(parent)->position()->transform().position();
}

Vec2Value* VertexTransformComponent::left() {
  if (parent) {
    HandleEntity* left = static_cast<VertexEntity*>(parent)->left();
    if (left) return &left->transform().position();
    return nullptr;
  }

  return nullptr;
}

Vec2Value* VertexTransformComponent::right() {
  if (parent) {
    HandleEntity* right = static_cast<VertexEntity*>(parent)->right();
    if (right) return &right->transform().position();
    return nullptr;
  }

  return nullptr;
}

Box VertexTransformComponent::bounding_box() const {
  Box box{ {0.0f, 0.0f}, {0.0f, 0.0f} };

  if (!parent) return box;

  vec2 position = m_position.get();
  HandleEntity* left = static_cast<VertexEntity*>(parent)->right();
  HandleEntity* right = static_cast<VertexEntity*>(parent)->right();

  if (left) {
    vec2 left_pos = left->transform().position().get();
    min(box.min, left_pos, box.min);
    max(box.max, left_pos, box.max);
  }

  if (right) {
    vec2 right_pos = right->transform().position().get();
    min(box.min, right_pos, box.min);
    max(box.max, right_pos, box.max);
  }

  box.min += position;
  box.max += position;

  return box;
}

void VertexTransformComponent::translate(const vec2& amount, bool apply) {
  static_cast<VertexEntity*>(parent)->position()->transform().translate(amount, apply);
}

void VertexTransformComponent::translate_to(const vec2& value, bool apply) {
  static_cast<VertexEntity*>(parent)->position()->transform().translate_to(value, apply);
}

void VertexTransformComponent::translate_left(const vec2& amount, bool mirror = false, bool apply = false) {
  if (!parent) return;

  HandleEntity* left = static_cast<VertexEntity*>(parent)->left();
  HandleEntity* right = static_cast<VertexEntity*>(parent)->right();

  if (!left) return;

  left->transform().translate(amount, apply);

  if (mirror && right) {
    vec2 direction = left->transform().position().get();
    normalize_length(direction, -1.0f, direction);

    if (!is_almost_zero(direction)) {
      float len = length(right->transform().position().get());
      translate_right_to(direction * len, false, apply);
    }
  }
}

void VertexTransformComponent::translate_right(const vec2& amount, bool mirror = false, bool apply = false) {
  if (!parent) return;

  HandleEntity* left = static_cast<VertexEntity*>(parent)->left();
  HandleEntity* right = static_cast<VertexEntity*>(parent)->right();

  if (!right) return;

  right->transform().translate(amount, apply);

  if (mirror && left) {
    vec2 direction = right->transform().position().get();
    normalize_length(direction, -1.0f, direction);

    if (!is_almost_zero(direction)) {
      float len = length(left->transform().position().get());
      translate_left_to(direction * len, false, apply);
    }
  }
}

void VertexTransformComponent::translate_left_to(const vec2& value, bool mirror = false, bool apply = false) {
  if (!parent) return;

  HandleEntity* left = static_cast<VertexEntity*>(parent)->left();
  HandleEntity* right = static_cast<VertexEntity*>(parent)->right();

  if (!left) return;

  left->transform().translate_to(value, apply);

  if (mirror && right) {
    vec2 direction = normalize_length(value, -1.0f);

    if (!is_almost_zero(direction)) {
      float len = length(right->transform().position().get());
      translate_right_to(direction * len, false, apply);
    }
  }
}

void VertexTransformComponent::translate_right_to(const vec2& value, bool mirror = false, bool apply = false) {
  if (!parent) return;

  HandleEntity* left = static_cast<VertexEntity*>(parent)->left();
  HandleEntity* right = static_cast<VertexEntity*>(parent)->right();

  if (!right) return;

  right->transform().translate_to(value, apply);

  if (mirror && left) {
    vec2 direction = normalize_length(value, -1.0f);

    if (!is_almost_zero(direction)) {
      float len = length(left->transform().position().get());
      translate_left_to(direction * len, false, apply);
    }
  }
}

void VertexTransformComponent::apply() {
  m_position.apply();

  HandleEntity* left = static_cast<VertexEntity*>(parent)->left();
  HandleEntity* right = static_cast<VertexEntity*>(parent)->right();

  if (left) left->transform().apply();
  if (right) right->transform().apply();
}

Box ElementTransformComponent::bounding_box() const {
  Box box{ std::numeric_limits<vec2>::max(), std::numeric_limits<vec2>::min() };
  ElementEntity* parent = static_cast<ElementEntity*>(this->parent);

  if (!parent) return box;

  vec2 position = m_position.get();

  if (parent->curves_count() > 0) {
    for (auto it = parent->curves_begin(); it != parent->curves_end(); ++it) {
      Box curve_box = it->bounding_box();
      min(box.min, curve_box.min, box.min);
      max(box.max, curve_box.max, box.max);
    }
  } else {
    box.min = { 0.0f, 0.0f };
    box.max = { 0.0f, 0.0f };
  }

  box.min += position;
  box.max += position;

  return box;
}

Box ElementTransformComponent::large_bounding_box() const {
  Box box = bounding_box();
  ElementEntity* parent = static_cast<ElementEntity*>(this->parent);
  vec2 position = m_position.get();

  box.min -= position;
  box.max -= position;

  for (auto& [id, vertex] : *parent) {
    Box vertex_box = vertex->transform().bounding_box();
    min(box.min, vertex_box.min, box.min);
    max(box.max, vertex_box.max, box.max);
  }

  box.min += position;
  box.max += position;

  return box;
}
