/**
 * @file action.h
 * @brief This file contains the action structure of the history manager.
 */

#pragma once

#include "../utils/uuid.h"

namespace Graphick::History {

  /**
   * @brief This structure represents an action that can be executed or reverted.
   *
   * An action includes the id of the entity that was affected and the type of action that was performed to allow multiplayer synchronization.
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
      Transform   /* The transform component. */
    };
  public:
    Type type;            /* The type of action that was performed. */
    Property property;    /* The property that was affected by the action. */

    uuid entity_id;       /* The id of the entity that was affected. */
  public:
    /**
     * @brief Constructs an action with the given type, entity id, property, data, and value.
     *
     * @param type The type of action that was performed.
     * @param entity_id The id of the entity that was affected.
     * @param property The property that was affected by the action.
     * @param data The new data to be set.
     * @param value The value that was modified.
     */
    template<typename T>
    Action(Type type, uuid entity_id, Property property, const T& data, T* value) :
      type(type),
      entity_id(entity_id),
      property(property),
      value(value),
      data(nullptr),
      backup(nullptr),
      size(sizeof(T))
    {
      this->data = new uint8_t[size];
      std::memcpy(this->data, &data, size);

      switch (type) {
      case Type::Add:
      case Type::Remove:
        break;
      case Type::Modify:
        this->backup = new uint8_t[size];
        std::memcpy(this->backup, value, size);
        break;
      }
    }

    /**
     * @brief Copy constructor, move constructor, and destructor.
     */
    Action(const Action& other);
    Action(Action&& other) noexcept;
    ~Action();

    /**
     * @brief Copy and move assignment operators.
     */
    Action& operator=(const Action& other);
    Action& operator=(Action&& other) noexcept;

    /**
     * @brief Executes the action.
     */
    void execute();

    /**
     * @brief Reverts the action.
     */
    void revert();

    /**
     * @brief Merges the action with the given action.
     *
     * Checks if the actions can be merged and if so, merges them. If the merge is successful, the given action is invalidated.
     * The merging process updates the data of the action to the new data.
     *
     * @param other The action to merge with.
     * @return true if the actions were merged, false otherwise.
     */
    bool merge(const Action& other);
  private:
    void* data;     /* The new data to be set. */
    void* backup;   /* The old data before the action was performed. */

    void* value;    /* A pointer to the value that the action is affecting, can also be retrieved from entity_id -> property. */
    size_t size;    /* The size of the value that was modified, used for heap operations. */
  };

}
