/**
 * @file action.cpp
 * @brief This file contains the implementation of the action structure of the history manager.
 */

#include "action.h"

#include "../editor/editor.h"

namespace Graphick::History {

  Action::Action(const Action& other) :
    type(other.type),
    entity_id(other.entity_id),
    property(other.property),
    value(other.value),
    data(nullptr),
    backup(nullptr),
    size(other.size)
  {
    data = new char[size];
    std::memcpy(data, other.data, size);

    if (type == Type::Modify) {
      backup = new char[size];
      std::memcpy(backup, other.backup, size);
    }
  }

  Action::Action(Action&& other) noexcept :
    type(other.type),
    entity_id(other.entity_id),
    property(other.property),
    value(other.value),
    data(other.data),
    backup(other.backup),
    size(other.size)
  {
    other.type = Type::Invalid;
    other.backup = nullptr;
    other.data = nullptr;
  }

  Action::~Action() {
    delete[] data;
    delete[] backup;
  }

  Action& Action::operator=(const Action& other) {
    if (this == &other) return *this;

    delete[] data;
    delete[] backup;

    type = other.type;
    entity_id = other.entity_id;
    property = other.property;
    value = other.value;
    data = nullptr;
    backup = nullptr;
    size = other.size;

    data = new char[size];
    std::memcpy(data, other.data, size);

    if (type == Type::Modify) {
      backup = new char[size];
      std::memcpy(backup, other.backup, size);
    }

    return *this;
  }

  Action& Action::operator=(Action&& other) noexcept {
    if (this == &other) return *this;

    delete[] data;
    delete[] backup;

    type = other.type;
    entity_id = other.entity_id;
    property = other.property;
    value = other.value;
    data = other.data;
    backup = other.backup;
    size = other.size;

    other.type = Type::Invalid;
    other.backup = nullptr;
    other.data = nullptr;

    return *this;
  }

  void Action::execute() {
    switch (type) {
    case Type::Add:
      break;
    case Type::Remove:
      // Editor::Editor::scene().remove(entity_id);
      break;
    case Type::Modify:
      std::memcpy(value, data, size);
      break;
    }
  }

  void Action::revert() {
    switch (type) {
    case Type::Add:
      break;
    case Type::Remove:
      // Editor::Editor::scene().add(entity_id, *(static_cast<const std::vector<uint8_t>*>(data)));
      break;
    case Type::Modify:
      std::memcpy(value, backup, size);
      break;
    }
  }

  bool Action::merge(const Action& other) {
    if (type != other.type || entity_id != other.entity_id || property != other.property) {
      return false;
    }

    switch (type) {
    case Type::Add:
      break;
    case Type::Remove:
      break;
    case Type::Modify:
      std::memcpy(data, other.data, size);
      break;
    }

    return true;
  }

}
