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

  void load() {
    m_children.insert({ {}, std::make_shared<Entity>() });
  }

  void render() {
    for (const auto& [uuid, entity] : m_children) {
      entity->render();
    }
  }
private:
  OrderedMap<UUID, std::shared_ptr<Entity>> m_children;
};
