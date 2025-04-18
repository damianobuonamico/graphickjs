/**
 * @file editor/scene/components/common.h
 * @brief Common utility classes and functions for the components.
 */

#pragma once

namespace graphick::io {

struct EncodedData;
struct DataDecoder;

}  // namespace graphick::io

namespace graphick::io::json {

class JSON;

};

namespace graphick::editor {

class Entity;

/**
 * @brief Base component wrapper struct.
 *
 * This struct is a wrapper around the actual component data, to allow for manipulation and history
 * tracking. Holds a pointer to the entity it belongs to.
 */
struct ComponentWrapper {
 public:
  /**
   * @brief Constructor.
   */
  ComponentWrapper(const Entity* entity) : m_entity(entity) {}

  /**
   * @brief Encodes the component in binary format.
   *
   * Each component is encoded as a sequence of bytes containing:
   *  - The component id.
   *  - The component bitfield containing the non-default fields (if more than one field).
   *  - The non-default fields in the order they are defined in the component data struct.
   *
   * @param data The encoded data to append the component to.
   * @return A reference to the encoded data.
   */
  virtual io::EncodedData& encode(io::EncodedData& data) const = 0;

  /**
   * @brief If possible encodes the component data in a JSON format.
   *
   * If the data already has a JSON representation of this component, it is either updated or
   * invalidated.
   *
   * @param data The JSON object to append the component data to.
   */
  virtual void ui_data(io::json::JSON& data) const {}

 protected:
  /**
   * @brief Modifies the underlying data of the component.
   *
   * @param decoder A diff of the modified component's data.
   */
  virtual void modify(io::DataDecoder& decoder) = 0;

 protected:
  const Entity* m_entity;  // A pointer to the entity this component belongs to.
};

}  // namespace graphick::editor
