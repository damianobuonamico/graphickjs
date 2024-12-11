/**
 * @file editor/scene/components/text.h
 * @brief Contains the TextComponent.
 */

#pragma once

#include "common.h"

#include "../../../math/rect.h"
#include "../../../utils/uuid.h"

#include <string>

namespace graphick::editor {

/**
 * @brief ImageComponent data.
 *
 * This struct should not be used directly, use the ImageComponent wrapper instead.
 */
struct TextData {
  std::string text = "Lorem ipsum";  // The text of the entity.
  uuid font_id = uuid::null;         // The UUID of the image data in the ResourceManager cache.

  TextData() = default;
  TextData(const std::string& text, const uuid font_id) : text(text), font_id(font_id) {}
  TextData(io::DataDecoder& decoder);

  inline operator renderer::Text() const
  {
    return {text, font_id};
  }

  /**
   * @brief Returns the bounding rect of the image.
   *
   * The bounding box method of a parent component is required to be implemented in the component's
   * data struct, not in the wrapper: the TransformComponent can only access the data struct.
   *
   * @return The bounding rect of the image.
   */
  rect bounding_rect() const;
};

/**
 * @brief TextComponent wrapper.
 *
 * An TextComponent is the base of the text entity.
 */
struct TextComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 8;  // The component id.

  using Data = TextData;                      // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  TextComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Returns the id of the image_data.
   *
   * @return The id of the image_data.
   */
  inline uuid font_id() const
  {
    return m_data->font_id;
  }

  /**
   * @brief Returns the image data of the entity.
   *
   * @return The image data of the entity.
   */
  inline const std::string& text() const
  {
    return m_data->text;
  }

  /**
   * @brief Returns the actual bounding rect of the text.
   *
   * @return The bounding rect of the text.
   */
  rect bounding_rect() const;

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
  void modify(io::DataDecoder& decoder) override {};

 private:
  Data* m_data;  // The actual component data.
 private:
  friend class Entity;
};

}  // namespace graphick::editor
