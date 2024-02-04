/**
 * @file history.h
 * @brief This file contains the history manager of the editor.
 */

#pragma once

#include "action.h"

#include <vector>

namespace Graphick::History {

  class History {
  public:
    History(const History&) = delete;
    History(History&&) = delete;

    /**
     * @brief Get the instance of the history manager.
     */
    static inline History* get() { return s_instance; }

    /**
     * @brief Initialize the history manager.
     *
     * This function should be called once before using the history manager.
     */
    static void init();

    /**
     * @brief Shutdown the history manager.
     *
     * This function should be called once before the application exits.
     */
    static void shutdown();

    /**
     * @brief Add a action to the history manager.
     *
     * @param type The type of the action.
     * @param entity_id The id of the entity the action is related to.
     * @param property The property of the entity the action is related to.
     * @param data The data of the action.
     */
    template<typename T>
    static void add(Action::Type type, uuid entity_id, Action::Property property, const T& data, T* value) {
      get()->add({ type, entity_id, property, data, value });
    }

    /**
     * @brief Undo the last action.
     */
    static void undo();

    /**
     * @brief Redo the last undone action.
     */
    static void redo();

    /**
     * @brief Pop the last action from the history.
     */
    static void pop();

    /**
     * @brief End the current batch of actions.
     */
    static void end_batch();
  private:
    History() = default;
    ~History() = default;

    /**
     * @brief Add a action to the history manager.
     *
     * It automatically executes the action and calls the seal method.
     *
     * @param action The action to add.
     */
    void add(Action action);

    /**
     * @brief Seal the history.
     *
     * This method clears the redo buffer;
     */
    void seal();

    /**
     * @brief Clear the history.
     */
    void clear();
  private:
    std::vector<Action> m_actions;    /* The list of actions. */

    int64_t m_index = -1;             /* The index of the last action. */
  private:
    static History* s_instance;       /* The instance of the history manager. */
  };

}
