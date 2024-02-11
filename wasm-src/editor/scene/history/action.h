/**
 * @file action.h
 * @brief This file contains the action struct of the history.
 * @todo use a unionS
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
   * An action includes the id and the property of the entity that was affected.
   * Includes the type of action that was performed to allow multiplayer synchronization.
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
      Add,        /* An entity or a component was added. */
      Remove,     /* An entity or a component was removed. */
      Modify      /* A property was modified. */
    };

    /**
     * @brief The property that was affected by the action.
     */
    enum class Property {
      Entity,     /* The entire entity. */
      Component,  /* A component. */
      Transform   /* The transform component. */
    };
  public:
    Type type;            /* The type of action that was performed. */
    Property property;    /* The property that was affected by the action. */

    uuid entity_id;       /* The id of the entity that was affected. */
  public:
    /**
     * @brief Constructs an action with the given entity id, property and type.
     *
     * @param entity_id The id of the entity that was affected.
     * @param property The property that was affected by the action.
     * @param type The type of action that was performed.
     */
    Action(uuid entity_id, Property property, Type type) :
      type(type),
      property(property),
      entity_id(entity_id) {}

    /**
     * @brief Executes the action.
     */
    virtual void execute(Scene* scene) const = 0;

    /**
     * @brief Reverts the action.
     */
    virtual void revert(Scene* scene) const = 0;

    /**
     * @brief Merges the action with the given action.
     *
     * Checks if the actions can be merged and if so, merges them. If the merge is successful, the given action is invalidated.
     * The merging process updates the data of the action to the new data.
     *
     * @param other The action to merge with.
     * @return true if the actions were merged, false otherwise.
     */
    virtual bool merge(const Action& other) = 0;
  };

  /**
   * @brief This struct represents an action that adds or removes an entity or a component.
   *
   * @struct AddOrRemoveAction
   */
  struct AddOrRemoveAction : public Action {
  public:
    /**
     * @brief Constructs an add or remove action with the given entity id, property and data.
     *
     * @param entity_id The id of the entity that was affected.
     * @param property The property that was affected by the action.
     * @param type The type of action, either Type::Add or Type::Remove.
     * @param data The encoded data of the property.
     */
    AddOrRemoveAction(uuid entity_id, Property property, Type type, const io::EncodedData& data);
    AddOrRemoveAction(uuid entity_id, Property property, Type type, io::EncodedData&& data);

    /**
     * @brief Executes the action.
     */
    void execute(Scene* scene) const override;

    /**
     * @brief Reverts the action.
     */
    void revert(Scene* scene) const override;

    /**
     * @brief Merges the action with the given action.
     *
     * Checks if the actions can be merged and if so, merges them. If the merge is successful, the given action is invalidated.
     * The merging process updates the data of the action to the new data.
     *
     * @param other The action to merge with.
     * @return true if the actions were merged, false otherwise.
     */
    bool merge(const Action& other) override;
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
     * @brief Reverts the add action.
     */
    void revert_add(Scene* scene) const;

    /**
     * @brief Reverts the remove action.
     */
    void revert_remove(Scene* scene) const;
  private:
    io::EncodedData m_data;    /* The encoded data of the property. */
  };

  /**
   * @brief This struct represents an action that adds or removes an entity or a component.
   *
   * @struct ModifyAction
   */
  struct ModifyAction : public Action {
  public:
    /**
     * @brief Constructs an add or remove action with the given entity id, property and data.
     *
     * @param entity_id The id of the entity that was affected.
     * @param property The property that was affected by the action.
     * @param data The data of the property.
     */
    template<typename T>
    ModifyAction(uuid entity_id, Property property, const T& data, T* value) :
      Action(entity_id, property, Type::Modify),
      m_data(nullptr),
      m_backup(nullptr),
      m_value(value),
      m_size(sizeof(T))
    {
      m_data = new uint8_t[m_size];
      m_backup = new uint8_t[m_size];

      std::memcpy(m_data, &data, m_size);
      std::memcpy(m_backup, value, m_size);
    }

    /**
     * @brief Copy constructor, move constructor, and destructor.
     */
    ModifyAction(const ModifyAction& other);
    ModifyAction(ModifyAction&& other) noexcept;
    ~ModifyAction();

    /**
     * @brief Copy and move assignment operators.
     */
    ModifyAction& operator=(const ModifyAction& other);
    ModifyAction& operator=(ModifyAction&& other) noexcept;

    /**
     * @brief Executes the action.
     */
    void execute(Scene* scene) const override;

    /**
     * @brief Reverts the action.
     */
    void revert(Scene* scene) const override;

    /**
     * @brief Merges the action with the given action.
     *
     * Checks if the actions can be merged and if so, merges them. If the merge is successful, the given action is invalidated.
     * The merging process updates the data of the action to the new data.
     *
     * @param other The action to merge with.
     * @return true if the actions were merged, false otherwise.
     */
    bool merge(const Action& other) override;
  private:
    void* m_data;      /* The new data to be set. */
    void* m_backup;    /* The old data before the action was performed. */

    void* m_value;     /* A pointer to the value that the action is affecting, can also be retrieved from entity_id -> property. */

    size_t m_size;     /* The size of the data. */
  };

}
