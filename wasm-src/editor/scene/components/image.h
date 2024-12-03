/**
 * @file editor/scene/components/image.h
 * @brief Contains the ImageComponent.
 */

#pragma once

#include "common.h"

#include "../../../geom/path.h"
#include "../../../utils/uuid.h"

namespace graphick::editor {

/**
 * @brief ImageComponent data.
 *
 * This struct should not be used directly, use the ImageComponent wrapper instead.
 */
struct ImageData {
  uuid image_id;  // The UUID of the image data in the ResourceManager cache.

  ImageData() = default;
  ImageData(const uuid image_id) : image_id(image_id) {}
  ImageData(io::DataDecoder& decoder);

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
 * @brief ImageComponent wrapper.
 *
 * An ImageComponent is the base of the image entity.
 */
struct ImageComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 7;  // The component id.
  using Data = ImageData;                     // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  ImageComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Returns the id of the image_data.
   *
   * @return The id of the image_data.
   */
  inline uuid id() const
  {
    return m_data->image_id;
  }

  /**
   * @brief Returns the image data of the entity.
   *
   * @return The image data of the entity.
   */
  const uint8_t* data() const;

  /**
   * @brief Returns the size of the image.
   *
   * @return The size of the image.
   */
  ivec2 size() const;

  /**
   * @brief Returns the number of channels of the image.
   *
   * @return The number of channels of the image.
   */
  uint8_t channels() const;

  /**
   * @brief Returns the outline path of the image.
   *
   * @return The outline path of the image.
   */
  geom::path path() const;

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
  void modify(io::DataDecoder& decoder) override {}

 private:
  Data* m_data;  // The actual component data.
 private:
  friend class Entity;
};

}  // namespace graphick::editor
