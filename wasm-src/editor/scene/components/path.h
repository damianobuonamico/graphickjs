/**
 * @file editor/scene/components/path.h
 * @brief Contains the PathComponent.
 */

#pragma once

#include "common.h"

#include "../../../geom/path.h"

namespace graphick::editor {

/**
 * @brief PathComponent data.
 *
 * This struct should not be used directly, use the PathComponent wrapper instead.
 */
using PathData = geom::path;

/**
 * @brief PathComponent wrapper.
 *
 * A path is a set of points and commands that define the shape of an element entity.
 */
struct PathComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 4;  // The component id.
  using Data = PathData;                      // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  PathComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to geom::path&.
   */
  inline operator const geom::path&() const
  {
    return *m_data;
  }

  /**
   * @brief Returns the path data of the entity.
   *
   * @return The path data of the entity.
   */
  inline const geom::path& data() const
  {
    return *m_data;
  }

  /**
   * @brief Returns the path data of the entity.
   *
   * @return The path data of the entity.
   */
  inline const geom::path* operator->() const
  {
    return m_data;
  }

  /**
   * @brief Moves the path cursor to the given point.
   *
   * @param p0 The point to move the cursor to.
   * @return The index of the newly added control point.
   */
  size_t move_to(const vec2 p0);

  /**
   * @brief Adds a line segment to the path.
   *
   * @param p1 The point to add to the path.
   * @param reverse Whether to add the point to the beginning of the path, instead of the end.
   * Default is false.
   * @return The index of the newly added control point.
   */
  size_t line_to(const vec2 p1, const bool reverse = false);

  /**
   * @brief Adds a quadratic bezier curve to the path.
   *
   * @param p1 The first control point of the curve.
   * @param p2 The second control point of the curve.
   * @param reverse Whether to add the point to the beginning of the path, instead of the end.
   * Default is false.
   * @return The index of the newly added control point.
   */
  size_t quadratic_to(const vec2 p1, const vec2 p2, const bool reverse = false);

  /**
   * @brief Adds a cubic bezier curve to the path.
   *
   * @param p1 The first control point of the curve.
   * @param p2 The second control point of the curve.
   * @param p3 The third control point of the curve.
   * @param reverse Whether to add the point to the beginning of the path, instead of the end.
   * Default is false.
   * @return The index of the newly added control point.
   */
  size_t cubic_to(const vec2 p1, const vec2 p2, const vec2 p3, const bool reverse = false);

  /**
   * @brief Closes the path by adding a segment to the first point in the path.
   *
   * If incoming or outgoing handles are present, the new segment will be a cubic bezier curve.
   *
   * @param reverse Whether to add the point to the beginning of the path, instead of the end.
   * Default is false.
   * @return The index of the newly added control point.
   */
  size_t close(const bool reverse = false);

  /**
   * @brief Translates a control point in the path by a given delta.
   *
   * @param point_index The index of the point to translate.
   * @param delta The translation delta.
   */
  void translate(const size_t point_index, const vec2 delta);

  /**
   * @brief Converts the given command to a line command.
   *
   * @param command_index The index of the command to convert.
   * @param reference_point The control point to return the updated index of.
   * @return The updated index of the reference point.
   */
  size_t to_line(const size_t command_index, const size_t reference_point = 0);

  /**
   * @brief Converts the given command to a cubic command.
   *
   * @param command_index The index of the command to convert.
   * @param reference_point The control point to return the updated index of.
   * @return The updated index of the reference point.
   */
  size_t to_cubic(const size_t command_index, const size_t reference_point = 0);

  /**
   * @brief Splits the segment at the given index at the given t value.
   *
   * @param segment_index The index of the segment to split.
   * @param t The t value to split the segment at.
   * @return The index of the newly added vertex.
   */
  size_t split(const size_t segment_index, const float t);

  /**
   * @brief Removes the ith control point from the path.
   *
   * @param point_index The index of the control point to remove.
   * @param keep_shape Whether to keep the shape of the path after removing the control point.
   * Default is false.
   */
  void remove(const size_t point_index, const bool keep_shape = false);

  /**
   * @brief Encodes the component in binary format.
   *
   * @param data The encoded data to append the component to.
   * @return A reference to the encoded data.
   */
  io::EncodedData& encode(io::EncodedData& data) const override;

 private:
  /**
   * @brief Path history modification types.
   */
  enum class PathModifyType { LoadData = 0, ModifyPoint = 1 << 0 };

 private:
  /**
   * @brief Modifies the underlying data of the component.
   *
   * @param decoder A diff of the modified component's data.
   */
  void modify(io::DataDecoder& decoder) override;

  /**
   * @brief Commits a PathModifyAction::LoadData to the history.
   *
   * @param action The action to commit.
   * @return An index returned by the action, can be ignored.
   */
  size_t commit_load(const std::function<size_t()> action);

 private:
  Data* m_data;  // The actual component data.
 private:
  friend class Entity;
};

}  // namespace graphick::editor
