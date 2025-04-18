/**
 * @file history.h
 * @brief This file contains the history of a scene.
 */

#pragma once

#include "action.h"

#include <variant>
#include <vector>

namespace graphick::editor {

/**
 * @brief This class represents the history of a scene.
 *
 * The history is a list of actions that can be undone and redone.
 */
class History {
 public:
  /**
   * @brief Default, move and copy constructors.
   */
  History(Scene* scene);
  History(const History&) = delete;
  History(History&&) = delete;

  /**
   * @brief Default constructor.
   */
  ~History() = default;

  /**
   * @brief Push an Add action to the history.
   *
   * @param entity_id The id of the entity the action is related to.
   * @param target The target of the entity the action is related to.
   * @param encoded_data The encoded data of the target.
   * @param execute Whether to execute the action or not (i.e. the action was already executed),
   */
  void add(uuid entity_id,
           Action::Target target,
           const io::EncodedData& encoded_data,
           const bool execute = true);
  void add(uuid entity_id,
           Action::Target target,
           io::EncodedData&& encoded_data,
           const bool execute = true);

  /**
   * @brief Push a Remove action to the history.
   *
   * @param entity_id The id of the entity the action is related to.
   * @param target The target of the entity the action is related to.
   * @param encoded_data The encoded data of the target.
   * @param execute Whether to execute the action or not (i.e. the action was already executed),
   */
  void remove(uuid entity_id,
              Action::Target target,
              const io::EncodedData& encoded_data,
              const bool execute = true);
  void remove(uuid entity_id,
              Action::Target target,
              io::EncodedData&& encoded_data,
              const bool execute = true);

  /**
   * @brief Push a Modify action to the history.
   *
   * @param entity_id The id of the entity the action is related to.
   * @param encoded_data The encoded data of the target.
   * @param backup_data The encoded backup data of the target.
   * @param execute Whether to execute the action or not (i.e. the action was already executed),
   * default is true.
   */
  void modify(uuid entity_id,
              const io::EncodedData& encoded_data,
              const io::EncodedData& backup_data,
              const bool execute = true);
  void modify(uuid entity_id,
              io::EncodedData&& encoded_data,
              io::EncodedData&& backup_data,
              const bool execute = true);

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
   * @brief Clear the history.
   */
  void clear();

  /**
   * @brief End the current batch of actions.
   */
  void end_batch();

 private:
  /**
   * @brief Push an action to the history.
   *
   * It automatically executes the action (if execute is set to true) and calls the seal method.
   *
   * @param action The action to add.
   * @param execute Whether to execute the action or just push it, default is true.
   */
  void push(Action&& action, const bool execute = true);

  /**
   * @brief Seal the history.
   *
   * This method clears the redo buffer;
   */
  void seal();

 private:
  std::vector<Action> m_actions;        // The list of actions.
  std::vector<size_t> m_batch_indices;  // The indices of the start of each batch.

  int64_t m_batch_index = 0;            // The index of the last batch.

  Scene* m_scene;                       // The scene the history is related to.
};

}  // namespace graphick::editor
