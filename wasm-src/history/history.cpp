/**
 * @file history.cpp
 * @brief This file contains the implementation of the history manager of the editor
 *
 * @todo batch actions
 */

#include "history.h"

#include "../utils/console.h"
#include "../utils/assert.h"

namespace Graphick::History {

  History* History::s_instance = nullptr;

  void History::init() {
    GK_ASSERT(!s_instance, "History already initialized, call shutdown() before reinitializing!");
    s_instance = new History();
  }

  void History::shutdown() {
    GK_ASSERT(s_instance, "History not initialized, call init() before shutting down!");
    delete s_instance;
    s_instance = nullptr;
  }

  void History::undo() {
    History* instance = get();

    if (instance->m_index >= 0 && !instance->m_actions.empty()) {
      instance->m_actions[instance->m_index].revert();
      instance->m_index--;
    }

    // console::log("-----------------------------------");
    // for (auto& action : instance->m_actions) {
    //   console::log("Action", (int)action.type);
    //   console::log("uuid", action.entity_id);
    //   console::log("-------");
    // }
  }

  void History::redo() {
    History* instance = get();
    size_t next_index = instance->m_index + 1;

    if (next_index < instance->m_actions.size() && next_index >= 0) {
      instance->m_actions[next_index].execute();
      instance->m_index++;
    }

    // console::log("-----------------------------------");
    // for (auto& action : instance->m_actions) {
    //   console::log("Action", (int)action.type);
    //   console::log("uuid", action.entity_id);
    //   console::log("-------");
    // }
  }

  void History::pop() {
    History* instance = get();

    instance->m_actions.erase(std::next(instance->m_actions.begin(), instance->m_index - 1), instance->m_actions.end());
    instance->m_index = static_cast<int64_t>(instance->m_actions.size()) - 1;
  }

  void History::end_batch() {

  }

  void History::add(Action action) {
    action.execute();

    // if (!get()->m_enabled) {
    //   return;
    // } else if (get()->m_ignore_next) {
    //   clear_ignore();
    //   return;
    // }

    seal();

    bool merged = false;

    if (!m_actions.empty()) {
      Action& last_action = m_actions.back();

      // if (last_action->can_merge() && action->can_merge()) {
      //   if (last_action->type == Command::Type::Batch) {
      //     CommandBatch* action_batch = static_cast<CommandBatch*>(last_action.get());
      //     action_batch->add(action);
      //     merged = true;
      //   } else {
      merged = last_action.merge(action);
  //   }
  // }
    }

    if (!merged) {
      m_actions.push_back(std::move(action));
      m_index++;
    }
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
