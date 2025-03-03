/**
 * @file common.h
 * @brief Contains common classes and functions used by input tools in the Graphick editor.
 */

#pragma once

#include "../keys.h"

#include "../../../geom/path.h"

#include "../../../math/mat2x3.h"

namespace graphick::editor {
struct PathComponent;
}

namespace graphick::editor::input {

/**
 * @brief Translates a control point of a path using the pen or direct selection tool.
 *
 * If create_handles is true, even if the control point provided is a vertex, it's incoming and
 * outgoing handles will be translated.
 *
 * @param path The path to move the control point on.
 * @param point_index The control point to move.
 * @param transform The transformation matrix of the element containing the path.
 * @param override_movement A pointer to a vec2 used to override the movement of the control point,
 * default is nullptr.
 * @param create_handles Whether to create handles if not present when moving the control point,
 * default is false.
 * @param keep_in_handle_length Whether to keep the length of the incoming handle when moving the
 * control point, default is true.
 * @param translate_in_first Whether to translate the incoming handle first, and in case calculate
 * the outgoing handle accordingly, default is false.
 * @param direction A pointer to an integer that will be set to the direction of the move (1 for
 * forward, -1 for backward), default is nullptr.
 * @param in_use_handle A pointer to a History::Vec2Value object that will be set to the handle
 * that is currently in use (out handle), default is nullptr.
 * @return The update point index (i.e. a segment was converted from linear to cubic, so two points
 * were added).
 */
size_t translate_control_point(PathComponent& path,
                               const size_t point_index,
                               const mat2x3& transform,
                               const vec2* override_movement = nullptr,
                               bool create_handles = false,
                               bool keep_in_handle_length = true,
                               bool translate_in_first = false,
                               int* direction = nullptr);

/**
 * @brief A class representing a selection rectangle.
 *
 * This class is used to represent a rectangular selection in the editor.
 * It is used by various tools to determine the area of the canvas that
 * should be affected by the tool's operation.
 */
class SelectionRect {
 public:
  /**
   * @brief Constructs a SelectionRect object.
   *
   * @param dashed A boolean value indicating whether the selection rectangle should be dashed or
   * not.
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
  inline bool active() const
  {
    return m_active;
  }

  /**
   * @brief Returns the position of the selection rectangle.
   *
   * @return A vec2 object representing the position of the selection rectangle.
   */
  inline vec2 position() const
  {
    return m_position;
  }

  /**
   * @brief Returns the path of the selection rectangle.
   *
   * @return A geom::path object representing the path of the selection rectangle.
   */
  inline const geom::path& path() const
  {
    return m_path;
  }

  /**
   * @brief Returns the transform matrix of the selection rectangle.
   *
   * @return A mat2x3 object representing the transform matrix of the selection rectangle.
   */
  mat2x3 transform() const;

  /**
   * @brief Returns the bounding rectangle of the selection rectangle.
   *
   * @return A rect object representing the bounding rectangle of the selection rectangle.
   */
  rect bounding_rect() const;

    /**
   * @brief Returns the rotated bounding rectangle of the selection rectangle.
   *
   * The rotated bounding rectangle follows the rotation of the selection rectangle.
   */
  rrect bounding_rrect() const;

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
  inline void size(const vec2 size)
  {
    m_size = size;
  }

  /**
   * @brief Sets the angle of the selection rectangle.
   *
   * @param angle A float value representing the new angle of the selection rectangle.
   */
  inline void angle(const float angle)
  {
    m_angle = angle;
  }

  /**
   * @brief Resets the selection rectangle to its default state.
   */
  void reset();

 protected:
  bool m_dashed = false;      /* Whether should be dashed or not. */
  bool m_active = false;      /* Whether is active or not. */

  vec2 m_position;            /* Position of the selection rectangle. */
  vec2 m_anchor_position;     /* Absolute anchor position of the selection rectangle. */
  vec2 m_size = {1.0f, 1.0f}; /* Size of the selection rectangle. */
  float m_angle = 0.0f;       /* Angle of the selection rectangle. */

  geom::path m_path;          /* The path to render. */
};

class Manipulator : public SelectionRect {
 public:
  /**
   * @brief An enum class representing the different types of handles that can be used to transform
   * an entity.
   */
  enum HandleType {
    /* Edge scale handles. */
    N,
    S,
    E,
    W,

    /* Corner scale handles. */
    NE,
    NW,
    SE,
    SW,

    /* Edge rotate handles. */
    RN,
    RS,
    RE,
    RW,

    /* Corner rotate handles. */
    RNE,
    RNW,
    RSE,
    RSW,

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
  inline bool in_use() const
  {
    return m_in_use;
  }

  /**
   * @brief Returns the positions of the manipulator handles.
   *
   * @return An array of vec2 objects representing the positions of the handles, it's size is equal
   * to HandleNone.
   */
  inline const vec2* handles() const
  {
    return m_handles;
  }

  /**
   * @brief Updates the state of the manipulator.
   *
   * @return A boolean value indicating whether the manipulator is active or not.
   */
  bool update();

  /**
   * @brief Pointer down event handler.
   *
   * @param threshold The virtual size of the handles.
   * @return A boolean value indicating whether the event was handled or not.
   */
  bool on_pointer_down(const float threshold);

  /**
   * @brief Pointer move event handler.
   *
   * If the shift key is pressed, uniform scaling or rotation snapping is triggered.
   */
  void on_pointer_move();

  /**
   * @brief Pointer up event handler.
   */
  void on_pointer_up();

  /**
   * @brief Key event handler.
   *
   * @param down Whether the key was pressed or released.
   * @param key The key that was pressed or released.
   * @return A boolean value indicating whether the event was handled or not.
   */
  bool on_key(const bool down, const KeyboardKey key);

 private:
  /**
   * @brief Updates the positions of the handles.
   *
   * @param bounding_rrect The target bounding rectangle of the manipulator.
   */
  void update_positions(const rrect& bounding_rrect);

  /**
   * @brief Scale pointer move event handler.
   *
   * If the shift key is pressed, uniform scaling is triggered.
   */
  void on_scale_pointer_move();

  /**
   * @brief Rotate pointer move event handler.
   *
   * If the shift key is pressed, rotation snapping is triggered.
   */
  void on_rotate_pointer_move();

 private:
  std::vector<mat2x3> m_cache;   // The cache of the transform matrices.
  vec2 m_handles[HandleNone];    // The positions of the handles.
  float m_threshold = 0.0f;      // The virtual size of the handles.
  bool m_in_use = false;         // Whether the manipulator is in use or not.
  vec2 m_center = {0.0f, 0.0f};  // The center of the manipulator.
  vec2 m_handle = {0.0f, 0.0f};  // The active handle start position.
  HandleType m_active_handle;    // The active handle. */

  rrect m_start_bounding_rrect;  // The start bounding rectangle of the manipulator.
  mat2x3 m_start_transform;      // The start transform matrix of the manipulator.
};

}  // namespace graphick::editor::input
