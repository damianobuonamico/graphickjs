#pragma once

#include "../entity.h"

class TextEntity: public Entity {
public:
  TextEntity(const vec2& position): m_transform(this, position) {};
  TextEntity(const TextEntity&) = default;
  TextEntity(TextEntity&&) = default;

  ~TextEntity() {}

  inline virtual TransformComponent* transform() override { return &m_transform; }
  inline const TransformComponent* transform() const override { return &m_transform; }

  virtual void render(const RenderingOptions& options) const override;
private:
  TransformComponent m_transform;
};
