/**
 * @file tool.h
 * @brief Contains the Tool class, which represents a tool used in the editor.
 */

#pragma once

#include "keys.h"

namespace Graphick::Editor::Input {

  /**
   * @brief The Tool class represents a tool used in the editor.
   *
   * It contains information about the type of tool, its category, and its behavior.
   * It is an abstract class, so it cannot be instantiated.
   *
   * @class Tool
   */
  class Tool {
  public:
    /**
     * @brief The ToolType enum represents the type of tool.
     */
    enum class ToolType {
      Pan = 0,
      Zoom,
      Select,
      DirectSelect,
      Pen,
      Pencil,
      None
    };

    /**
     * @brief The Category enum represents the category of the tool.
     */
    enum Category {
      CategoryNone = 0,
      CategoryDirect = 1 << 0,
      CategoryImmediate = 1 << 1,
    };
  public:
    /**
     * Deleted default constructor, copy constructor, move constructor, and destructor.
     */
    Tool() = delete;
    Tool(const Tool&) = delete;
    Tool(Tool&&) = delete;
    ~Tool() = default;

    /**
     * @brief Returns the type of the tool.
     *
     * @return ToolType The type of the tool.
     */
    inline ToolType type() const { return m_type; }

    /**
     * @brief Returns the category of the tool.
     *
     * @return int The category of the tool.
     */
    inline int category() const { return m_category; }

    /**
     * @brief Checks if the tool is in the given category.
     *
     * @param category The category to check.
     *
     * @return true If the tool is in the given category.
     * @return false If the tool is not in the given category.
     */
    inline bool is_in_category(Category category) const { return m_category & category; }

    /**
     * @brief Called when the pointer is pressed down.
     */
    virtual void on_pointer_down() {}

    /**
     * @brief Called when the pointer is moved.
     */
    virtual void on_pointer_move() {}

    /**
     * @brief Called when the pointer is released.
     */
    virtual void on_pointer_up() {}

    /**
     * @brief Called when the pointer is hovering.
     */
    virtual void on_pointer_hover() {}

    /**
     * @brief Called when a key is pressed or released.
     *
     * @param down Whether the key is pressed down or released.
     * @param key The key that was pressed or released.
     */
    virtual void on_key(const bool down, const KeyboardKey key) {}

    /**
     * @brief Resets the tool to its default state.
     */
    virtual void reset() {}

    /**
     * @brief Renders any overlays for the tool.
     */
    virtual void render_overlays() const {}
  protected:
    /**
     * @brief Constructs a new Tool object with the given type and category.
     *
     * @param type The type of the tool.
     * @param category The category of the tool.
     */
    Tool(ToolType type, int category) : m_type(type), m_category(category) {}
  protected:
    const ToolType m_type;    /* The type of the tool. */
    const int m_category;     /* The category of the tool. */
  };

}
