#pragma once

#include "../../values/ordered_map.h"
#include "../../utils/uuid.h"
#include "entity.h"

#include <memory>

class Scene {
public:
  Scene() = default;
  Scene(const Scene&) = default;
  Scene(Scene&&) = default;

  ~Scene() = default;

  void add_entity(const std::shared_ptr<Entity>& entity) {
    m_children.insert({ entity->id, entity });
  }
private:
  void load() {
    m_children.insert({ {}, std::make_shared<Entity>() });
  }

  void render(float zoom) {
    for (const auto& [uuid, entity] : m_children) {
      entity->render(zoom);
    }
  }
private:
  OrderedMap<UUID, std::shared_ptr<Entity>> m_children;

  friend class Editor;
};
