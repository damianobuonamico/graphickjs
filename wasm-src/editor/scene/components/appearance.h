/**
 * @file editor/scene/components/appearance.h
 * @brief Contains the components that define the appearance of the entity.
 */

#pragma once

#include "common.h"

#include "../../../math/vec4.h"
#include "../../../renderer/properties.h"
#include "../../../utils/uuid.h"

namespace graphick::editor {

/**
 * @brief FillComponent data.
 *
 * This struct should not be used directly, use the FillComponent wrapper instead.
 */
struct FillData {
  renderer::Paint paint = vec4{0.0f, 0.0f, 0.0f, 1.0f};   // The fill color.
  renderer::FillRule rule = renderer::FillRule::NonZero;  // The fill rule.

  bool visible = true;                                    // Whether or not to display the fill.

  FillData() = default;
  FillData(const vec4& color) : paint(color) {}
  FillData(io::DataDecoder& decoder);

  /**
   * @brief Conversion operator to renderer::Fill.
   */
  inline operator renderer::Fill() const
  {
    return renderer::Fill(paint, rule);
  }
};

/**
 * @brief FillComponent wrapper.
 *
 * A stroke is a collection of proprerties used for rendering.
 */
struct FillComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 5;  // The component id.
  using Data = FillData;                      // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  FillComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to renderer::Fill.
   */
  operator renderer::Fill() const
  {
    return fill();
  }

  /**
   * @brief Returns the data as a renderer::Fill.
   *
   * @return The data as a renderer::Fill.
   */
  const renderer::Fill fill() const
  {
    return renderer::Fill(paint(), rule());
  }

  /**
   * @brief Returns the paint data of the fill.
   *
   * @return The paint data of the fill.
   */
  const renderer::Paint& paint() const
  {
    return m_data->paint;
  }

  /**
   * @brief Returns the fill rule of the fill.
   *
   * @return The fill rule of the fill.
   */
  renderer::FillRule rule() const
  {
    return m_data->rule;
  }

  /**
   * @brief Returns whether the fill is visible.
   *
   * @return true if the fill is visible, false otherwise.
   */
  bool visible() const
  {
    return m_data->visible;
  }

  /**
   * @brief Encodes the component in binary format.
   *
   * @param data The encoded data to append the component to.
   * @return A reference to the encoded data.
   */
  io::EncodedData& encode(io::EncodedData& data) const override;

 private:
  /**
   * @brief Modifies the underlying data of the component.
   *
   * @param decoder A diff of the modified component's data.
   */
  void modify(io::DataDecoder& decoder) override;

 private:
  Data* m_data;  // The actual component data.
 private:
  friend class Entity;
};

/**
 * @brief StrokeComponent data.
 *
 * This struct should not be used directly, use the StrokeComponent wrapper instead.
 */
struct StrokeData {
  renderer::Paint paint = vec4{0.0f, 0.0f, 0.0f, 1.0f};  // The stroke color.
  renderer::LineCap cap = renderer::LineCap::Butt;       // The line cap.
  renderer::LineJoin join = renderer::LineJoin::Miter;   // The line join.

  float miter_limit = 10.0f;  // The miter limit, only used if join is set to miter.
  float width = 1.0f;         // The line width.

  bool visible = true;        // Whether or not to display the stroke.

  StrokeData() = default;
  StrokeData(const vec4& color) : paint(color) {}
  StrokeData(const vec4& color, const float width) : paint(color), width(width) {}
  StrokeData(io::DataDecoder& decoder);

  /**
   * @brief Conversion operator to renderer::Stroke.
   */
  inline operator renderer::Stroke() const
  {
    return renderer::Stroke(paint, cap, join, width, miter_limit);
  }
};

/**
 * @brief StrokeComponent wrapper.
 *
 * A stroke is a collection of proprerties used for rendering.
 */
struct StrokeComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 6;  // The component id.
  using Data = StrokeData;                    // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  StrokeComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to renderer::Stroke.
   */
  operator renderer::Stroke() const
  {
    return stroke();
  }

  /**
   * @brief Returns the data as a renderer::Stroke.
   *
   * @return The data as a renderer::Stroke.
   */
  const renderer::Stroke stroke() const
  {
    return renderer::Stroke(paint(), cap(), join(), width(), miter_limit());
  }

  /**
   * @brief Returns the paint data of the stroke.
   *
   * @return The paint data of the stroke.
   */
  const renderer::Paint& paint() const
  {
    return m_data->paint;
  }

  /**
   * @brief Returns the line cap of the stroke.
   *
   * @return The line cap of the stroke.
   */
  renderer::LineCap cap() const
  {
    return m_data->cap;
  }

  /**
   * @brief Returns the line join of the stroke.
   *
   * @return The line join of the stroke.
   */
  renderer::LineJoin join() const
  {
    return m_data->join;
  }

  /**
   * @brief Returns the miter limit of the stroke.
   *
   * @return The miter limit of the stroke.
   */
  float miter_limit() const
  {
    return m_data->miter_limit;
  }

  /**
   * @brief Returns the width of the stroke.
   *
   * @return The width of the stroke.
   */
  float width() const
  {
    return m_data->width;
  }

  /**
   * @brief Returns whether the fill is visible.
   *
   * @return true if the fill is visible, false otherwise.
   */
  bool visible() const
  {
    return m_data->visible;
  }

  /**
   * @brief Encodes the component in binary format.
   *
   * @param data The encoded data to append the component to.
   * @return A reference to the encoded data.
   */
  io::EncodedData& encode(io::EncodedData& data) const override;

 private:
  /**
   * @brief Modifies the underlying data of the component.
   *
   * @param decoder A diff of the modified component's data.
   */
  void modify(io::DataDecoder& decoder) override;

 private:
  Data* m_data;  // The actual component data.
 private:
  friend class Entity;
};

}  // namespace graphick::editor
