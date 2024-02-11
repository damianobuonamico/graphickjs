/**
 * @file history.h
 * @brief This file contains the history of a scene.
 */

#pragma once

#include "action.h"

#include <vector>
#include <variant>

namespace Graphick::Editor {

  class History {
  public:
    using ActionVariant = std::variant<AddOrRemoveAction, ModifyAction>;
  public:
    History(Scene* scene);
    History(const History&) = delete;
    History(History&&) = delete;

    ~History() = default;

    /**
     * @brief Push an add action to the history.
     *
     * @param entity_id The id of the entity the action is related to.
     * @param property The property of the entity the action is related to.
     * @param data The encoded data of the property.
     */
    void add(uuid entity_id, Action::Property property, const io::EncodedData& encoded_data);
    void add(uuid entity_id, Action::Property property, io::EncodedData&& encoded_data);

    /**
     * @brief Push a remove action to the history.
     *
     * @param entity_id The id of the entity the action is related to.
     * @param property The property of the entity the action is related to.
     * @param data The encoded data of the property.
     */
    void remove(uuid entity_id, Action::Property property, const io::EncodedData& encoded_data);
    void remove(uuid entity_id, Action::Property property, io::EncodedData&& encoded_data);

    /**
     * @brief Push a modify action to the history.
     *
     * @param type The type of the action.
     * @param entity_id The id of the entity the action is related to.
     * @param property The property of the entity the action is related to.
     * @param data The data of the action.
     */
    template<typename T>
    void modify(uuid entity_id, Action::Property property, const T& data, T* value) {
      push(ModifyAction{
        entity_id,
        property,
        data,
        value
      });
    }

    /**
     * @brief Undo the last action.
     */
    void undo();

    /**
     * @brief Redo the last undone action.
     */
    void redo();

    /**
     * @brief Pop the last action from the history.
     */
    void pop();

    /**
     * @brief End the current batch of actions.
     */
    void end_batch();
  private:
    /**
     * @brief Push an action to the history.
     *
     * It automatically executes the action and calls the seal method.
     *
     * @param action The action to add.
     */
    void push(ActionVariant&& action);

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
    std::vector<ActionVariant> m_actions;    /* The list of actions. */

    int64_t m_index = -1;                    /* The index of the last action. */

    Scene* m_scene;                          /* The scene the history is related to. */
  };

}
