/**
 * @file common.h
 * @brief Contains common classes and functions used by input tools in the Graphick editor.
 */
#pragma once

#include "../../../renderer/geometry/internal.h"

namespace Graphick::History {
  class Mat2x3Value;
}

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
     * @brief An enum class representing the different types of handles that can be used to transform an entity.
     */
    enum HandleType {
      N, S, E, W,                 /* Edge scale handles. */
      NE, NW, SE, SW,             /* Corner scale handles. */
      RN, RS, RE, RW,             /* Edge rotate handles. */
      RNE, RNW, RSE, RSW,         /* Corner rotate handles. */
      HandleNone
    };
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
     * @brief Returns whether the tool is currently in use.
     *
     * @return true if the tool is in use, false otherwise.
     */
    inline bool in_use() const { return m_in_use; }

    /**
     * @brief Returns the positions of the manipulator handles.
     *
     * @return An array of vec2 objects representing the positions of the handles, it's size is equal to HandleNone.
     */
    inline const vec2* handles() const { return m_handles; }

    /**
     * @brief Updates the state of the manipulator.
     *
     * @return A boolean value indicating whether the manipulator is active or not.
     */
    bool update();

    /**
     * @brief Pointer down event handler.
     *
     * @param position The position of the pointer.
     * @param threshold The virtual size of the handles.
     * @return A boolean value indicating whether the event was handled or not.
     */
    bool on_pointer_down(const vec2 position, const float threshold);

    /**
     * @brief Pointer move event handler.
     *
     * @param position The position of the pointer.
     */
    void on_pointer_move(const vec2 position);

    /**
     * @brief Pointer up event handler.
     */
    void on_pointer_up();
  private:
    /**
     * @brief Updates the positions of the handles.
     *
     * @param bounding_rect The target bounding rectangle of the manipulator.
     */
    void update_positions(const rect& bounding_rect);
  private:
    std::vector<History::Mat2x3Value*> m_cache;   /* The cache of the transform matrices. */
    vec2 m_handles[HandleNone];                   /* The positions of the handles. */

    bool m_in_use = false;                        /* Whether the manipulator is in use or not. */
    vec2 m_center = { 0.0f, 0.0f };               /* The center of the manipulator. */
    vec2 m_handle = { 0.0f, 0.0f };               /* The active handle start position. */
    HandleType m_active_handle;                   /* The active handle. */

    rect m_start_bounding_rect;                   /* The start bounding rectangle of the manipulator. */
  };

}
