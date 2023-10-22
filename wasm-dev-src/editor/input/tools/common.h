/**
 * @file common.h
 * @brief Contains common classes and functions used by input tools in the Graphick editor.
 */
#pragma once

#include "../../../renderer/geometry/internal.h"

namespace Graphick::Editor::Input {

  /**
   * @brief A class representing a selection rectangle.
   *
   * This class is used to represent a rectangular selection in the editor.
   * It is used by various tools to determine the area of the canvas that
   * should be affected by the tool's operation.
   *
   * @class SelectionRect
   */
  class SelectionRect {
  public:
    /**
     * @brief Constructs a SelectionRect object.
     *
     * @param dashed A boolean value indicating whether the selection rectangle should be dashed or not.
     */
    SelectionRect(bool dashed = false);

    /**
     * @brief Copy constructor for SelectionRect.
     *
     * @param other The SelectionRect object to copy.
     */
    SelectionRect(const SelectionRect& other) = default;

    /**
     * @brief Move constructor for SelectionRect.
     *
     * @param other The SelectionRect object to move from.
     */
    SelectionRect(SelectionRect&&) = default;

    /**
     * @brief Default destructor for SelectionRect.
     */
    ~SelectionRect() = default;

    /**
     * @brief Returns whether the selection rectangle is active or not.
     *
     * @return A boolean value indicating whether the selection rectangle is active or not.
     */
    inline bool active() const { return m_active; }

    /**
     * @brief Returns the position of the selection rectangle.
     *
     * @return A vec2 object representing the position of the selection rectangle.
     */
    inline vec2 position() const { return m_position; }

    /**
     * @brief Returns the path of the selection rectangle.
     *
     * @return A Renderer::Geometry::Internal::PathInternal object representing the path of the selection rectangle.
     */
    inline Renderer::Geometry::Internal::PathInternal path() const { return m_path; }

    /**
     * @brief Returns the bounding rectangle of the selection rectangle.
     *
     * @return A rect object representing the bounding rectangle of the selection rectangle.
     */
    rect bounding_rect() const;

    /**
     * @brief Sets the position of the selection rectangle.
     *
     * @param position A vec2 object representing the new position of the selection rectangle.
     */
    void set(const vec2 position);

    /**
     * @brief Sets the size of the selection rectangle.
     *
     * @param size A vec2 object representing the new size of the selection rectangle.
     */
    void size(const vec2 size);

    /**
     * @brief Resets the selection rectangle to its default state.
     */
    void reset();
  protected:
    bool m_dashed = false;                                /* Whether should be dashed or not. */
    bool m_active = false;                                /* Whether is active or not. */

    vec2 m_position;                                      /* Position of the selection rectangle. */
    vec2 m_anchor_position;                               /* Absolute anchor position of the selection rectangle. */

    Renderer::Geometry::Internal::PathInternal m_path;    /* The path to render. */
  };

  class Manipulator : public SelectionRect {
  public:
    /**
     * @brief Constructs a Manipulator object.
     */
    Manipulator() : SelectionRect(false) {}

    /**
     * @brief Copy constructor for Manipulator.
     *
     * @param other The Manipulator object to copy.
     */
    Manipulator(const Manipulator& other) = default;

    /**
     * @brief Move constructor for Manipulator.
     *
     * @param other The Manipulator object to move from.
     */
    Manipulator(Manipulator&&) = default;

    /**
     * @brief Default destructor for Manipulator.
     */
    ~Manipulator() = default;

    /**
     * @brief Updates the state of the manipulator.
     *
     * @return A boolean value indicating whether the manipulator is active or not.
     */
    bool update();
  };

}
