/**
 * @file action.h
 * @brief This file contains the action struct of the history.
 */

#pragma once

#include "../../../utils/uuid.h"

#include "../../../io/encode/encode.h"

#include <vector>

namespace Graphick::Editor {

  class Scene;

  /**
   * @brief This struct represents an action that can be executed or reverted.
   *
   * An action includes the id and the target of the entity that was affected.
   * A modify action can only be performed on a component, and affects one or more of its properties.
   * A property is always present in a component. To "remove" a property, a modify action with its default value is performed.
   *
   * @struct Action
   */
  struct Action {
  public:
    /**
     * @brief The type of action that was performed.
     */
    enum class Type {
      Invalid,    /* Invalid action. An action is invalidated when moved. */
      Add,        /* An entity or component was added. */
      Remove,     /* An entity or component was removed. */
      Modify      /* A component was modified. */
    };

    /**
     * @brief The target that was affected by the action.
     */
    enum class Target {
      Entity,     /* The entire entity. */
      Component,  /* A component of the entity. */
    };
  public:
    Type type;            /* The type of action that was performed. */
    Target target;        /* The target of the action. */

    uuid entity_id;       /* The id of the entity that was affected. */
  public:
    /**
     * @brief Constructs an Add or Remove action with the given entity id, target, type and data.
     *
     * @param entity_id The id of the entity that was affected.
     * @param target The target that was affected by the action.
     * @param type The type of action that was performed, either Add or Remove.
     * @param data The encoded data of the target.
     */
    Action(uuid entity_id, Target target, Type type, const io::EncodedData& data);
    Action(uuid entity_id, Target target, Type type, io::EncodedData&& data);

    /**
     * @brief Constructs a Modify action with the given entity id, target, type, data and backup.
     *
     * @param entity_id The id of the entity that was affected.
     * @param target The target that was affected by the action.
     * @param type The type of action that was performed, only Modify is allowed for this constructor.
     * @param data The encoded data of the target.
     * @param backup The backup data of the target.
     */
    Action(uuid entity_id, Target target, Type type, const io::EncodedData& data, const io::EncodedData& backup);
    Action(uuid entity_id, Target target, Type type, io::EncodedData&& data, io::EncodedData&& backup);

    /**
     * @brief Copy and move constructors.
     */
    Action(const Action& other);
    Action(Action&& other) noexcept;

    /**
     * @brief Copy and move assignment operators.
     */
    Action& operator=(const Action& other);
    Action& operator=(Action&& other) noexcept;

    /**
     * @brief Executes the action.
     */
    void execute(Scene* scene) const;

    /**
     * @brief Reverts the action.
     */
    void revert(Scene* scene) const;

    /**
     * @brief Merges the action with the given action.
     *
     * Checks if the actions can be merged and if so, merges them. If the merge is successful, the given action is invalidated.
     * The merging process updates the data of the action to the new data.
     *
     * @param other The action to merge with.
     * @return true if the actions were merged, false otherwise.
     */
    bool merge(Action& other);
  private:
    /**
     * @brief Executes the add action.
     */
    void execute_add(Scene* scene) const;

    /**
     * @brief Executes the remove action.
     */
    void execute_remove(Scene* scene) const;

    /**
     * @brief Executes the modify action.
     */
    void execute_modify(Scene* scene) const;

    /**
     * @brief Reverts the add action.
     */
    void revert_add(Scene* scene) const;

    /**
     * @brief Reverts the remove action.
     */
    void revert_remove(Scene* scene) const;

    /**
     * @brief Reverts the modify action.
     */
    void revert_modify(Scene* scene) const;
  private:
    io::EncodedData m_data;      /* The encoded data of the target. */
    io::EncodedData m_backup;    /* The backup data of the target. */
  };

}
