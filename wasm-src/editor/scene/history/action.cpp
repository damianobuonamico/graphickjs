/**
 * @file action.cpp
 * @brief This file contains the implementation of the action struct of the history.
 *
 * @todo merging algorithm
 */

#include "action.h"

#include "../entity.h"

#include "../../../math/vector.h"

#include "../../../utils/assert.h"
#include "../../../utils/console.h"

namespace graphick::editor {

Action::Action(uuid entity_id, Target target, Type type, const io::EncodedData& data) :
  type(type), target(target), entity_id(entity_id), m_data(data) {
  GK_ASSERT(type != Type::Modify, "No backup data provided for Modify action!");
}

Action::Action(uuid entity_id, Target target, Type type, io::EncodedData&& data) :
  type(type), target(target), entity_id(entity_id), m_data(std::move(data)) {
  GK_ASSERT(type != Type::Modify, "No backup data provided for Modify action!");
}

Action::Action(uuid entity_id, Target target, Type type, const io::EncodedData& data, const io::EncodedData& backup) :
  type(type), target(target), entity_id(entity_id), m_data(data), m_backup(backup) {
  GK_ASSERT(type == Type::Modify, "Add or Remove actions cannot have backup data!");
  GK_ASSERT(target == Target::Component, "Modify actions can only target components!");
}

Action::Action(uuid entity_id, Target target, Type type, io::EncodedData&& data, io::EncodedData&& backup) :
  type(type), target(target), entity_id(entity_id), m_data(std::move(data)), m_backup(std::move(backup)) {
  GK_ASSERT(type == Type::Modify, "Add or Remove actions cannot have backup data!");
  GK_ASSERT(target == Target::Component, "Modify actions can only target components!");
}

Action::Action(const Action& other) :
  type(other.type),
  target(other.target),
  entity_id(other.entity_id),
  m_data(other.m_data),
  m_backup(other.type == Type::Modify ? other.m_backup : io::EncodedData()) { }

Action::Action(Action&& other) noexcept :
  type(other.type),
  target(other.target),
  entity_id(other.entity_id),
  m_data(std::move(other.m_data)),
  m_backup(other.type == Type::Modify ? std::move(other.m_backup) : io::EncodedData()) {
  other.type = Type::Invalid;
}

Action& Action::operator=(const Action& other) {
  if (this == &other) return *this;

  type = other.type;
  target = other.target;
  entity_id = other.entity_id;
  m_data = other.m_data;
  m_backup = other.type == Type::Modify ? other.m_backup : io::EncodedData();

  return *this;
}

Action& Action::operator=(Action&& other) noexcept {
  if (this == &other) return *this;

  type = other.type;
  target = other.target;
  entity_id = other.entity_id;
  m_data = std::move(other.m_data);
  m_backup = other.type == Type::Modify ? std::move(other.m_backup) : io::EncodedData();

  other.type = Type::Invalid;

  return *this;
}

void Action::execute(Scene* scene) const {
  switch (type) {
  case Type::Add:
    execute_add(scene);
    break;
  case Type::Remove:
    execute_remove(scene);
    break;
  case Type::Modify:
    execute_modify(scene);
    break;
  case Type::Invalid:
  default:
    break;
  }
}

void Action::revert(Scene* scene) const {
  switch (type) {
  case Type::Add:
    revert_add(scene);
    break;
  case Type::Remove:
    revert_remove(scene);
    break;
  case Type::Modify:
    revert_modify(scene);
    break;
  case Type::Invalid:
  default:
    break;
  }
}

bool Action::merge(Action& other) {
  if (entity_id != other.entity_id || type != Type::Modify || other.type != Type::Modify || target != other.target) {
    return false;
  }

  if (target == Target::Entity) {
    m_data = std::move(other.m_data);
    return true;
  }

  io::DataDecoder this_decoder(&m_data);
  io::DataDecoder other_decoder(&other.m_data);

  const uint8_t component_id = this_decoder.component_id();

  if (component_id != other_decoder.component_id()) {
    return false;
  }

  if (component_id != PathComponent::component_id) {
    m_data = std::move(other.m_data);
    return true;
  }

  const uint8_t modify_type = this_decoder.uint8();

  if (modify_type != other_decoder.uint8()) {
    return false;
  }

  const uint32_t point_index = this_decoder.uint32();

  if (point_index != other_decoder.uint32()) {
    return false;
  }

  m_data = std::move(other.m_data);
  return true;
}

void Action::execute_add(Scene* scene) const {
  rect bounding_rect;

  if (target == Target::Entity) {
    scene->add(entity_id, m_data);
    bounding_rect = scene->get_entity(entity_id).get_component<TransformComponent>().approx_bounding_rect();
    // console::log("add entity");
  } else {
    Entity entity = scene->get_entity(entity_id);
    rect bounding_rect_before = entity.get_component<TransformComponent>().approx_bounding_rect();
    entity.add(m_data);
    rect bounding_rect_after = entity.get_component<TransformComponent>().approx_bounding_rect();
    // console::log("add component");
    bounding_rect = rect{
      math::min(bounding_rect_before.min, bounding_rect_after.min),
      math::max(bounding_rect_before.max, bounding_rect_after.max)
    };
  }

  scene->m_cache.invalidate_rect(bounding_rect);
}

void Action::execute_remove(Scene* scene) const {
  rect bounding_rect;

  if (target == Target::Entity) {
    bounding_rect = scene->get_entity(entity_id).get_component<TransformComponent>().approx_bounding_rect();
    scene->remove(entity_id);
    // console::log("remove entity");
  } else {
    Entity entity = scene->get_entity(entity_id);
    rect bounding_rect_before = entity.get_component<TransformComponent>().approx_bounding_rect();
    entity.remove(m_data);
    rect bounding_rect_after = entity.get_component<TransformComponent>().approx_bounding_rect();
    // console::log("remove component");
    bounding_rect = rect{
      math::min(bounding_rect_before.min, bounding_rect_after.min),
      math::max(bounding_rect_before.max, bounding_rect_after.max)
    };
  }

  scene->m_cache.invalidate_rect(bounding_rect);
}

void Action::execute_modify(Scene* scene) const {
  Entity entity = scene->get_entity(entity_id);
  rect bounding_rect_before = entity.get_component<TransformComponent>().approx_bounding_rect();
  entity.modify(m_data);
  rect bounding_rect_after = entity.get_component<TransformComponent>().approx_bounding_rect();
  rect bounding_rect = rect{
    math::min(bounding_rect_before.min, bounding_rect_after.min),
    math::max(bounding_rect_before.max, bounding_rect_after.max)
  };

  scene->m_cache.invalidate_rect(bounding_rect);
}

void Action::revert_add(Scene* scene) const {
  rect bounding_rect;

  if (target == Target::Entity) {
    bounding_rect = scene->get_entity(entity_id).get_component<TransformComponent>().approx_bounding_rect();
    scene->remove(entity_id);
    // console::log("revert add entity");
  } else {
    Entity entity = scene->get_entity(entity_id);
    rect bounding_rect_before = entity.get_component<TransformComponent>().approx_bounding_rect();
    scene->get_entity(entity_id).remove(m_data);
    rect bounding_rect_after = entity.get_component<TransformComponent>().approx_bounding_rect();
    // console::log("revert add component");
    bounding_rect = rect{
      math::min(bounding_rect_before.min, bounding_rect_after.min),
      math::max(bounding_rect_before.max, bounding_rect_after.max)
    };
  }

  scene->m_cache.invalidate_rect(bounding_rect);
}

void Action::revert_remove(Scene* scene) const {
  rect bounding_rect;

  if (target == Target::Entity) {
    scene->add(entity_id, m_data);
    bounding_rect = scene->get_entity(entity_id).get_component<TransformComponent>().approx_bounding_rect();
    // console::log("revert remove entity");
  } else {
    Entity entity = scene->get_entity(entity_id);
    rect bounding_rect_before = entity.get_component<TransformComponent>().approx_bounding_rect();
    entity.add(m_data);
    rect bounding_rect_after = entity.get_component<TransformComponent>().approx_bounding_rect();
    // console::log("revert remove component");
    bounding_rect = rect{
      math::min(bounding_rect_before.min, bounding_rect_after.min),
      math::max(bounding_rect_before.max, bounding_rect_after.max)
    };
  }

  scene->m_cache.invalidate_rect(bounding_rect);
}

void Action::revert_modify(Scene* scene) const {
  Entity entity = scene->get_entity(entity_id);
  rect bounding_rect_before = entity.get_component<TransformComponent>().approx_bounding_rect();
  entity.modify(m_backup);
  rect bounding_rect_after = entity.get_component<TransformComponent>().approx_bounding_rect();
  rect bounding_rect = rect{
    math::min(bounding_rect_before.min, bounding_rect_after.min),
    math::max(bounding_rect_before.max, bounding_rect_after.max)
  };

  scene->m_cache.invalidate_rect(bounding_rect);
}

}  // namespace graphick::editor
