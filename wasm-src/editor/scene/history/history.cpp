/**
 * @file history.cpp
 * @brief This file contains the implementation of the history manager of the editor
 *
 * @todo batch actions
 */

#include "history.h"

namespace Graphick::Editor {

  History::History(Scene* scene) :
    m_scene(scene),
    m_index(-1) {}

  void History::add(uuid entity_id, Action::Property property, const std::vector<uint8_t>& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Add,
      encoded_data
    });
  }

  void History::add(uuid entity_id, Action::Property property, std::vector<uint8_t>&& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Add,
      encoded_data
    });
  }

  void History::remove(uuid entity_id, Action::Property property, const std::vector<uint8_t>& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Remove,
      encoded_data
    });
  }

  void History::remove(uuid entity_id, Action::Property property, std::vector<uint8_t>&& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Remove,
      encoded_data
    });
  }

  void History::undo() {
    if (m_index >= 0 && !m_actions.empty()) {
      std::visit([this](const auto& action) { action.revert(m_scene); }, m_actions[m_index]);
      m_index--;
    }
  }

  void History::redo() {
    size_t next_index = m_index + 1;

    if (next_index < m_actions.size() && next_index >= 0) {
      std::visit([this](const auto& action) { action.execute(m_scene); }, m_actions[next_index]);
      m_index++;
    }
  }

  void History::pop() {
    m_actions.erase(std::next(m_actions.begin(), m_index - 1), m_actions.end());
    m_index = static_cast<int64_t>(m_actions.size()) - 1;
  }

  void History::end_batch() {

  }

  void History::push(ActionVariant&& action) {
    bool merged = false;

    std::visit([this, &merged](auto& new_action) {
      new_action.execute(m_scene);

      seal();

      if (!m_actions.empty()) {
        // merged = last_action.merge(static_cast<Action&>(new_action));

        std::visit([&merged, &new_action](auto& last_action) {
          merged = last_action.merge(static_cast<Action&>(new_action));
        }, m_actions.back());
      }

      // if (last_action->can_merge() && action->can_merge()) {
      //   if (last_action->type == Command::Type::Batch) {
      //     CommandBatch* action_batch = static_cast<CommandBatch*>(last_action.get());
      //     action_batch->add(action);
      //     merged = true;
      //   } else {
    //   }
    // }

      }, action);

    if (!merged) {
      m_actions.push_back(std::move(action));
      m_index++;
    }
    // if (!get()->m_enabled) {
    //   return;
    // } else if (get()->m_ignore_next) {
    //   clear_ignore();
    //   return;
    // }
  }

  void History::seal() {
    if (m_index < static_cast<int64_t>(m_actions.size()) - 1) {
      m_actions.erase(m_actions.begin() + (m_index + 1), m_actions.end());
      m_index = static_cast<int64_t>(m_actions.size()) - 1;
    }
  }

  void History::clear() {
    m_actions.clear();
    m_index = -1;
  }

}
