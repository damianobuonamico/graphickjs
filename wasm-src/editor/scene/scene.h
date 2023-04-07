#pragma once

#include "../../values/map_value.h"
#include "../../utils/uuid.h"
#include "entity.h"
#include "selection_state.h"

#include <memory>

class Scene {
public:
  SelectionState selection;
public:
  Scene() = default;
  Scene(const Scene&) = default;
  Scene(Scene&&) = default;

  ~Scene() = default;

  inline void add_entity(const std::shared_ptr<Entity>& entity) {
    m_children.insert({ entity->id, entity });
  }

  Entity* entity_at(const vec2& position, bool lower_level, float threshold);
  std::vector<Entity*> entities_in(const Box& box, bool lower_level = false);

  Entity* duplicate(const Entity* entity);
private:
  void load();

  void render(float zoom, const Box& viewport) const;
  void render_selection(const RenderingOptions& options) const;
private:
  MapValue<UUID, std::shared_ptr<Entity>> m_children;
private:
  friend class Editor;
};
