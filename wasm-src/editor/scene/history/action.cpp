/**
 * @file action.cpp
 * @brief This file contains the implementation of the action struct of the history.
 *
 * @todo merging algorithm
 */

#include "action.h"

#include "../entity.h"

#include "../../../utils/console.h"
#include "../../../utils/assert.h"

namespace Graphick::Editor {

  Action::Action(uuid entity_id, Target target, Type type, const io::EncodedData& data) :
    type(type),
    target(target),
    entity_id(entity_id),
    m_data(data)
  {
    GK_ASSERT(type != Type::Modify, "No backup data provided for Modify action!");
  }

  Action::Action(uuid entity_id, Target target, Type type, io::EncodedData&& data) :
    type(type),
    target(target),
    entity_id(entity_id),
    m_data(std::move(data))
  {
    GK_ASSERT(type != Type::Modify, "No backup data provided for Modify action!");
  }

  Action::Action(uuid entity_id, Target target, Type type, const io::EncodedData& data, const io::EncodedData& backup) :
    type(type),
    target(target),
    entity_id(entity_id),
    m_data(data),
    m_backup(backup)
  {
    GK_ASSERT(type == Type::Modify, "Add or Remove actions cannot have backup data!");
    GK_ASSERT(target == Target::Component, "Modify actions can only target components!");
  }

  Action::Action(uuid entity_id, Target target, Type type, io::EncodedData&& data, io::EncodedData&& backup) :
    type(type),
    target(target),
    entity_id(entity_id),
    m_data(std::move(data)),
    m_backup(std::move(backup))
  {
    GK_ASSERT(type == Type::Modify, "Add or Remove actions cannot have backup data!");
    GK_ASSERT(target == Target::Component, "Modify actions can only target components!");
  }

  Action::Action(const Action& other) :
    type(other.type),
    target(other.target),
    entity_id(other.entity_id),
    m_data(other.m_data),
    m_backup(other.type == Type::Modify ? other.m_backup : io::EncodedData()) {}

  Action::Action(Action&& other) noexcept :
    type(other.type),
    target(other.target),
    entity_id(other.entity_id),
    m_data(std::move(other.m_data)),
    m_backup(other.type == Type::Modify ? std::move(other.m_backup) : io::EncodedData())
  {
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
    }
  }

  bool Action::merge(const Action& other) {
    if (
      entity_id != other.entity_id ||
      type != Type::Modify || other.type != Type::Modify ||
      target != other.target
    ) {
      return false;
    }

    m_data = std::move(other.m_data);

    return true;
  }

  void Action::execute_add(Scene* scene) const {
    if (target == Target::Entity) {
      scene->add(entity_id, m_data);
      console::log("add entity");
    } else {
      scene->get_entity(entity_id).add(m_data);
      console::log("add component");
    }
  }

  void Action::execute_remove(Scene* scene) const {
    if (target == Target::Entity) {
      scene->remove(entity_id);
      console::log("remove entity");
    } else {
      scene->get_entity(entity_id).remove(m_data);
      console::log("remove component");
    }
  }

  void Action::execute_modify(Scene* scene) const {
    scene->get_entity(entity_id).modify(m_data);
  }

  void Action::revert_add(Scene* scene) const {
    if (target == Target::Entity) {
      scene->remove(entity_id);
      console::log("revert add entity");
    } else {
      scene->get_entity(entity_id).remove(m_data);
      console::log("revert add component");
    }
  }

  void Action::revert_remove(Scene* scene) const {
    if (target == Target::Entity) {
      scene->add(entity_id, m_data);
      console::log("revert remove entity");
    } else {
      scene->get_entity(entity_id).add(m_data);
      console::log("revert remove component");
    }
  }

  void Action::revert_modify(Scene* scene) const {
    scene->get_entity(entity_id).modify(m_backup);
  }

}
