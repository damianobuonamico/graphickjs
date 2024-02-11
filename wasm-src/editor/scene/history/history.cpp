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
    m_batch_indices({ 0 }) {}

  void History::add(uuid entity_id, Action::Property property, const io::EncodedData& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Add,
      encoded_data
    });
  }

  void History::add(uuid entity_id, Action::Property property, io::EncodedData&& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Add,
      encoded_data
    });
  }

  void History::remove(uuid entity_id, Action::Property property, const io::EncodedData& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Remove,
      encoded_data
    });
  }

  void History::remove(uuid entity_id, Action::Property property, io::EncodedData&& encoded_data) {
    push(AddOrRemoveAction{
      entity_id,
      property,
      Action::Type::Remove,
      encoded_data
    });
  }

  void History::undo() {
    if (!m_actions.empty() && !m_batch_indices.empty() && m_batch_index > 0) {
      int64_t batch_start = m_batch_indices[m_batch_index - 1];
      int64_t batch_end = m_batch_indices[m_batch_index];

      for (int64_t i = batch_end - 1; i >= batch_start; i--) {
        std::visit([this](const auto& action) { action.revert(m_scene); }, m_actions[i]);
      }

      m_batch_index--;
    }
  }

  void History::redo() {
    size_t batch_start = m_batch_indices[m_batch_index];

    if (batch_start < m_actions.size()) {
      size_t batch_end = m_batch_index >= m_batch_indices.size() - 1 ? m_actions.size() : m_batch_indices[m_batch_index + 1];

      for (size_t i = batch_start; i < batch_end; i++) {
        std::visit([this](const auto& action) { action.execute(m_scene); }, m_actions[i]);
      }

      m_batch_index++;
    }
  }

  void History::pop() {
    if (m_actions.empty() || m_batch_indices.empty()) return;
    if (m_batch_indices.size() == 1) clear();

    m_actions.erase(m_actions.begin() + m_batch_indices[m_batch_indices.size() - 2], m_actions.end());
    m_batch_indices.pop_back();

    if (m_batch_index > m_batch_indices.size() - 1) m_batch_index = m_batch_indices.size() - 1;
  }

  void History::end_batch() {
    if (!m_batch_indices.empty() && m_batch_indices.back() == m_actions.size()) {
      return;
    }

    m_batch_indices.push_back(m_actions.size());
    m_batch_index++;
  }

  void History::push(ActionVariant&& action) {
    bool merged = false;

    std::visit([this, &merged](auto& new_action) {
      new_action.execute(m_scene);

      seal();

      if (!m_actions.empty() && !m_batch_indices.empty()) {
        if (m_batch_indices.back() < m_actions.size()) {
          /* There are other actions in the current batch. */
          std::visit([this, &merged, &new_action](auto& last_action) {

            size_t i = m_batch_indices.back();
            while (!merged && i < m_actions.size()) {
              /* Try to merge the new action with this action from the batch. */

              std::visit([this, &merged, &new_action](auto& batched_action) {
                merged = batched_action.merge(static_cast<Action&>(new_action));
              }, m_actions[i]);

              i++;
            }
          }, m_actions.back());
        }
      }
    }, action);

    if (!merged) {
      m_actions.push_back(std::move(action));
    }
  }

  void History::seal() {
    if (m_batch_index + 1 < m_batch_indices.size()) {
      m_actions.erase(m_actions.begin() + m_batch_indices[m_batch_index], m_actions.end());
      m_batch_indices.erase(m_batch_indices.begin() + m_batch_index + 1, m_batch_indices.end());
    }
  }

  void History::clear() {
    m_actions.clear();
    m_batch_indices = { 0 };
    m_batch_index = 0;
  }

}
