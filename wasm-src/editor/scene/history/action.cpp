/**
 * @file action.cpp
 * @brief This file contains the implementation of the action struct of the history.
 */

#include "action.h"

#include "../scene.h"

#include "../../../utils/console.h"
#include "../../../utils/assert.h"

namespace Graphick::Editor {

  AddOrRemoveAction::AddOrRemoveAction(
    uuid entity_id,
    Property property,
    Type type,
    const io::EncodedData& data
  ) :
    Action(entity_id, property, type),
    m_data(data)
  {
    GK_ASSERT(type == Type::Add || type == Type::Remove, "In an AddOrRemoveAction, type can only be either Add or Remove!");
  }

  AddOrRemoveAction::AddOrRemoveAction(
    uuid entity_id,
    Property property,
    Type type,
    io::EncodedData&& data
  ) :
    Action(entity_id, property, type),
    m_data(std::move(data))
  {
    GK_ASSERT(type == Type::Add || type == Type::Remove, "In an AddOrRemoveAction, type can only be either Add or Remove!");
  }

  void AddOrRemoveAction::execute(Scene* scene) const {
    if (type == Type::Add) execute_add(scene);
    else if (type == Type::Remove) execute_remove(scene);
  }

  void AddOrRemoveAction::revert(Scene* scene) const {
    if (type == Type::Add) revert_add(scene);
    else if (type == Type::Remove) revert_remove(scene);
  }

  bool AddOrRemoveAction::merge(const Action& other) {
    return false;
  }

  void AddOrRemoveAction::execute_add(Scene* scene) const {
    switch (property) {
    case Property::Entity:
      scene->add(entity_id, m_data);
      console::log("add entity");
      break;
    default:
      break;
    }
  }

  void AddOrRemoveAction::execute_remove(Scene* scene) const {
    switch (property) {
    case Property::Entity:
      scene->remove(entity_id);
      console::log("remove entity");
      break;
    default:
      break;
    }
  }

  void AddOrRemoveAction::revert_add(Scene* scene) const {
    switch (property) {
    case Property::Entity:
      scene->remove(entity_id);
      console::log("revert add entity");
      break;
    default:
      break;
    }
  }

  void AddOrRemoveAction::revert_remove(Scene* scene) const {
    switch (property) {
    case Property::Entity:
      scene->add(entity_id, m_data);
      console::log("revert remove entity");
      break;
    default:
      break;
    }
  }

  ModifyAction::ModifyAction(const ModifyAction& other) :
    Action(other.entity_id, other.property, Type::Modify),
    m_value(other.m_value),
    m_size(other.m_size)
  {
    m_data = new uint8_t[other.m_size];
    m_backup = new uint8_t[other.m_size];

    std::memcpy(m_data, other.m_data, other.m_size);
    std::memcpy(m_backup, other.m_backup, other.m_size);
  }

  ModifyAction::ModifyAction(ModifyAction&& other) noexcept :
    Action(other.entity_id, other.property, Type::Modify),
    m_value(other.m_value),
    m_data(other.m_data),
    m_backup(other.m_backup),
    m_size(other.m_size)
  {
    other.type = Type::Invalid;
    other.m_backup = nullptr;
    other.m_data = nullptr;
  }

  ModifyAction::~ModifyAction() {
    delete[] m_data;
    delete[] m_backup;
  }

  ModifyAction& ModifyAction::operator=(const ModifyAction& other) {
    if (this == &other) return *this;

    delete[] m_data;
    delete[] m_backup;

    type = Type::Modify;
    entity_id = other.entity_id;
    property = other.property;

    m_value = other.m_value;
    m_data = new uint8_t[other.m_size];
    m_backup = new uint8_t[other.m_size];
    m_size = other.m_size;

    std::memcpy(m_data, other.m_data, other.m_size);
    std::memcpy(m_backup, other.m_backup, other.m_size);

    return *this;
  }

  ModifyAction& ModifyAction::operator=(ModifyAction&& other) noexcept {
    if (this == &other) return *this;

    delete[] m_data;
    delete[] m_backup;

    type = Type::Modify;
    entity_id = other.entity_id;
    property = other.property;

    m_value = other.m_value;
    m_data = other.m_data;
    m_backup = other.m_backup;
    m_size = other.m_size;

    other.type = Type::Invalid;
    other.m_backup = nullptr;
    other.m_data = nullptr;

    return *this;
  }

  void ModifyAction::execute(Scene* scene) const {
    std::memcpy(m_value, m_data, m_size);
  }

  void ModifyAction::revert(Scene* scene) const {
    std::memcpy(m_value, m_backup, m_size);
  }

  bool ModifyAction::merge(const Action& other) {
    if (other.type != Type::Modify || entity_id != other.entity_id || property != other.property) {
      return false;
    }

    std::memcpy(m_data, static_cast<const ModifyAction&>(other).m_data, m_size);

    return true;
  }

}
