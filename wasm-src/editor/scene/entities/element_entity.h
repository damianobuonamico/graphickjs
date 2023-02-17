#pragma once

#include "../entity.h"
#include "bezier_entity.h"
#include "vertex_entity.h"
#include "handle_entity.h"
#include "../../../values/map_value.h"
#include "../../../values/bool_value.h"

#include <vector>
#include <memory>

class ElementEntity: public Entity {
public:
  ElementEntity(const vec2& position): Entity(CategorySelectable | CategorySelectableChildren), m_transform({ this, position }), m_selection({ this }) {
    add_vertex(std::make_shared<VertexEntity>(vec2{ 0.0f, 0.0f }, vec2{ 20.0f, -20.0f }, false));
    add_vertex(std::make_shared<VertexEntity>(vec2{ 100.0f, 0.0f }, vec2{ -20.0f, -20.0f }, true));
    add_vertex(std::make_shared<VertexEntity>(vec2{ 100.0f, 100.0f }, vec2{ 20.0f, 20.0f }, true));
    add_vertex(std::make_shared<VertexEntity>(vec2{ 0.0f, 100.0f }));

    console::log("ElementEntity created");
  };
  ElementEntity(const ElementEntity&) = default;
  ElementEntity(ElementEntity&&) = default;

  ~ElementEntity() {
    console::log("ElementEntity destroyed");
  }

  inline std::vector<BezierEntity>::const_iterator curves_begin() const { return m_curves.begin(); }
  inline std::vector<BezierEntity>::const_iterator curves_end() const { return m_curves.end(); }

  inline MapValue<UUID, std::shared_ptr<VertexEntity>>::iterator begin() { return m_vertices.begin(); }
  inline MapValue<UUID, std::shared_ptr<VertexEntity>>::iterator end() { return m_vertices.end(); }
  inline MapValue<UUID, std::shared_ptr<VertexEntity>>::const_iterator begin() const { return m_vertices.begin(); }
  inline MapValue<UUID, std::shared_ptr<VertexEntity>>::const_iterator end() const { return m_vertices.end(); }

  inline virtual ElementTransformComponent& transform() override { return m_transform; }
  inline virtual const ElementTransformComponent& transform() const override { return m_transform; }
  inline SelectionComponent* selection() { return &m_selection; }
  inline const SelectionComponent* selection() const { return &m_selection; }

  inline size_t vertex_count() const { return m_vertices.size(); }
  inline size_t curves_count() const { return m_curves.size(); }
  inline VertexEntity& first_vertex() const { return *m_vertices.begin()->second; }
  inline VertexEntity& last_vertex() const { return *(--m_vertices.end())->second; }

  void add_vertex(const std::shared_ptr<VertexEntity>& vertex);

  virtual void render(float zoom) const override;

  bool intersects_box(const Box& box) const;

  virtual Entity* entity_at(const vec2& position, bool lower_level, float threshold) override;
  virtual void entities_in(const Box& box, std::vector<Entity*>& entities, bool lower_level) override;

  // TODO: duplication (after implementing saving/loading)
  inline virtual ElementEntity* duplicate() const override { return new ElementEntity(*this); };
private:
  void regenerate();
  const BezierEntity closing_curve() const;
private:
  MapValue<UUID, std::shared_ptr<VertexEntity>> m_vertices;
  std::vector<BezierEntity> m_curves;

  ElementTransformComponent m_transform;
  SelectionComponent m_selection;

  BoolValue m_closed{ true };
};
