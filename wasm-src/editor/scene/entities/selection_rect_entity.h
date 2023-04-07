#pragma once

#include "../entity.h"

class SelectionRectEntity: public Entity {
public:
  SelectionRectEntity(bool dashed = false)
    : Entity(), m_transform(RectTransformComponent{ this, vec2{}, vec2{} }), m_dashed(dashed) {};
  SelectionRectEntity(const SelectionRectEntity&) = default;
  SelectionRectEntity(SelectionRectEntity&&) = default;

  ~SelectionRectEntity() = default;

  inline virtual RectTransformComponent* transform() override { return &m_transform; }
  inline const RectTransformComponent* transform() const override { return &m_transform; }

  inline bool active() const { return m_active; }

  void set(const vec2& position);
  void size(const vec2& size);
  void reset();

  virtual void tessellate_outline(const vec4& color, const RenderingOptions& options, Geometry& geo) const override;
  virtual void render(const RenderingOptions& options) const override;
private:
  bool m_dashed;
  bool m_active = false;

  RectTransformComponent m_transform;
};
