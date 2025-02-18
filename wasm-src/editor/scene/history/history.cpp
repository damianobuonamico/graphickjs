/**
 * @file history.cpp
 * @brief This file contains the implementation of the history manager of the editor
 */

#include "history.h"

#include "../scene.h"

#include "../../../utils/console.h"

namespace graphick::editor {

History::History(Scene* scene) : m_scene(scene), m_batch_indices({0}) {}

void History::add(uuid entity_id,
                  Action::Target target,
                  const io::EncodedData& encoded_data,
                  const bool execute)
{
  push(Action{entity_id, target, Action::Type::Add, encoded_data}, execute);
}

void History::add(uuid entity_id,
                  Action::Target target,
                  io::EncodedData&& encoded_data,
                  const bool execute)
{
  push(Action{entity_id, target, Action::Type::Add, std::move(encoded_data)}, execute);
}

void History::remove(uuid entity_id,
                     Action::Target target,
                     const io::EncodedData& encoded_data,
                     const bool execute)
{
  push(Action{entity_id, target, Action::Type::Remove, encoded_data}, execute);
}

void History::remove(uuid entity_id,
                     Action::Target target,
                     io::EncodedData&& encoded_data,
                     const bool execute)
{
  push(Action{entity_id, target, Action::Type::Remove, std::move(encoded_data)}, execute);
}

void History::modify(uuid entity_id,
                     const io::EncodedData& encoded_data,
                     const io::EncodedData& backup_data,
                     const bool execute)
{
  push(
      Action{
          entity_id, Action::Target::Component, Action::Type::Modify, encoded_data, backup_data},
      execute);
}

void History::modify(uuid entity_id,
                     io::EncodedData&& encoded_data,
                     io::EncodedData&& backup_data,
                     const bool execute)
{
  push(
      Action{
          entity_id, Action::Target::Component, Action::Type::Modify, encoded_data, backup_data},
      execute);
}

void History::undo()
{
  if (!m_actions.empty() && !m_batch_indices.empty() && m_batch_index > 0) {
    int64_t batch_start = m_batch_indices[m_batch_index - 1];
    int64_t batch_end = m_batch_indices[m_batch_index];

    for (int64_t i = batch_end - 1; i >= batch_start; i--) {
      m_actions[i].revert(m_scene);
    }

    m_batch_index--;
  }
}

void History::redo()
{
  size_t batch_start = m_batch_indices[m_batch_index];

  if (batch_start < m_actions.size()) {
    size_t batch_end = m_batch_index >= static_cast<int64_t>(m_batch_indices.size()) - 1 ?
                           m_actions.size() :
                           m_batch_indices[m_batch_index + 1];

    for (size_t i = batch_start; i < batch_end; i++) {
      m_actions[i].execute(m_scene);
    }

    m_batch_index++;
  }
}

void History::pop()
{
  if (m_actions.empty() || m_batch_indices.empty())
    return;
  if (m_batch_indices.size() == 1)
    clear();

  m_actions.erase(m_actions.begin() + m_batch_indices[m_batch_indices.size() - 2],
                  m_actions.end());
  m_batch_indices.pop_back();

  if (m_batch_index > static_cast<int64_t>(m_batch_indices.size()) - 1)
    m_batch_index = m_batch_indices.size() - 1;
}

void History::end_batch()
{
  if (!m_batch_indices.empty() && m_batch_indices.back() == m_actions.size()) {
    return;
  }

  m_batch_indices.push_back(m_actions.size());
  m_batch_index++;
}

void History::push(Action&& action, const bool execute)
{
  bool merged = false;

  if (execute) {
    action.execute(m_scene);
  } else {
    m_scene->m_cache.clear(action.entity_id);
  }

  seal();

  if (!m_actions.empty() && !m_batch_indices.empty()) {
    if (m_batch_indices.back() < m_actions.size()) {
      /* There are other actions in the current batch. */
      size_t i = m_batch_indices.back();

      while (!merged && i < m_actions.size()) {
        /* Try to merge the new action with this action from the batch. */
        merged = m_actions[i].merge(static_cast<Action&>(action));

        i++;
      }
    }
  }

  if (!merged) {
    m_actions.push_back(std::move(action));
  }
}

void History::seal()
{
  if (m_batch_index + 1 < static_cast<int64_t>(m_batch_indices.size())) {
    m_actions.erase(m_actions.begin() + m_batch_indices[m_batch_index], m_actions.end());
    m_batch_indices.erase(m_batch_indices.begin() + m_batch_index + 1, m_batch_indices.end());
  }
}

void History::clear()
{
  m_actions.clear();
  m_batch_indices = {0};
  m_batch_index = 0;
}

}  // namespace graphick::editor
