/**
 * @file editor/scene/components/group.h
 * @brief Contains the base components used by groups and layers in the editor.
 */

#pragma once

#include "common.h"

#include "../../../math/rect.h"
#include "../../../math/vec4.h"
#include "../../../utils/uuid.h"

#include <vector>

namespace entt {

enum class entity : uint32_t;

}

namespace graphick::editor {

class Scene;

/**
 * @brief GroupComponent data.
 *
 * This struct should not be used directly, use the GroupComponent wrapper instead.
 */
struct GroupData {
  enum class GroupType {
    Normal = 0,                        // A normal group.
    CompoundPath,                      // A compound path group.
  };

  std::vector<entt::entity> children;  // The ids of the children entities.

  GroupData() = default;
  GroupData(const std::vector<entt::entity>& children) : children(children) {}
  GroupData(io::DataDecoder& decoder);

  /**
   * @brief Returns the bounding rect of the group.
   *
   * The bounding box method of a parent component is required to be implemented in the component's
   * data struct, not in the wrapper: the TransformComponent can only access the data struct.
   *
   * @param scene The scene the group belongs to.
   * @return The bounding rect of the group.
   */
  rect bounding_rect(const Scene* scene) const;
};

/**
 * @brief GroupComponent wrapper.
 *
 * Once an GroupComponent is created, it cannot be modified.
 */
struct GroupComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 9;  // The component id.
  using Data = GroupData;                     // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  GroupComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to vector.
   */
  inline operator const std::vector<entt::entity>&() const
  {
    return m_data->children;
  }

  /**
   * @brief Iterator to the beginning of the children vector.
   *
   * @return The iterator to the beginning of the children vector.
   */
  inline std::vector<entt::entity>::const_iterator begin() const
  {
    return m_data->children.begin();
  }

  /**
   * @brief Iterator to the end of the children vector.
   *
   * @return The iterator to the end of the children vector.
   */
  inline std::vector<entt::entity>::const_iterator end() const
  {
    return m_data->children.end();
  }

  /**
   * @brief Iterator to the beginning of the children vector.
   *
   * @return The iterator to the beginning of the children vector.
   */
  inline std::vector<entt::entity>::const_reverse_iterator rbegin() const
  {
    return m_data->children.rbegin();
  }

  /**
   * @brief Iterator to the end of the children vector.
   *
   * @return The iterator to the end of the children vector.
   */
  inline std::vector<entt::entity>::const_reverse_iterator rend() const
  {
    return m_data->children.rend();
  }

  /**
   * @brief Adds an entity to the group.
   */
  inline void push_back(const entt::entity entity)
  {
    m_data->children.push_back(entity);
  }

  /**
   * @brief Removes the entity from the group.
   */
  inline void remove(const entt::entity entity)
  {
    m_data->children.erase(std::remove(m_data->children.begin(), m_data->children.end(), entity),
                           m_data->children.end());
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
 * @brief LayerComponent data.
 *
 * This struct should not be used directly, use the LayerComponent wrapper instead.
 */
struct LayerData {
  std::vector<entt::entity> children;  // The ids of the children entities.

  vec4 color;                          // The layer color.

  LayerData() = default;
  LayerData(const std::vector<entt::entity>& children) : children(children) {}
  LayerData(const vec4& color) : color(color) {}
  LayerData(io::DataDecoder& decoder);
};

/**
 * @brief LayerComponent wrapper.
 *
 * Once an LayerComponent is created, it cannot be modified.
 */
struct LayerComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 10;  // The component id.
  using Data = LayerData;                      // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  LayerComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to vector.
   */
  inline operator const std::vector<entt::entity>&() const
  {
    return m_data->children;
  }

  /**
   * @brief Iterator to the beginning of the children vector.
   *
   * @return The iterator to the beginning of the children vector.
   */
  inline std::vector<entt::entity>::const_iterator begin() const
  {
    return m_data->children.begin();
  }

  /**
   * @brief Iterator to the end of the children vector.
   *
   * @return The iterator to the end of the children vector.
   */
  inline std::vector<entt::entity>::const_iterator end() const
  {
    return m_data->children.end();
  }

  /**
   * @brief Iterator to the beginning of the children vector.
   *
   * @return The iterator to the beginning of the children vector.
   */
  inline std::vector<entt::entity>::const_reverse_iterator rbegin() const
  {
    return m_data->children.rbegin();
  }

  /**
   * @brief Iterator to the end of the children vector.
   *
   * @return The iterator to the end of the children vector.
   */
  inline std::vector<entt::entity>::const_reverse_iterator rend() const
  {
    return m_data->children.rend();
  }

  /**
   * @brief Returns the layer color.
   *
   * @return The layer color.
   */
  inline vec4 color() const
  {
    return m_data->color;
  }

  /**
   * @brief Adds an entity to the layer.
   */
  inline void push_back(const entt::entity entity)
  {
    m_data->children.push_back(entity);
  }

  /**
   * @brief Removes the entity from the layer.
   */
  inline void remove(const entt::entity entity)
  {
    m_data->children.erase(std::remove(m_data->children.begin(), m_data->children.end(), entity),
                           m_data->children.end());
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
 * @brief ArtboardComponent data.
 *
 * This struct should not be used directly, use the ArtboardComponent wrapper instead.
 */
struct ArtboardData {
  vec4 color;  // The artboard color.

  ArtboardData() = default;
  ArtboardData(const vec4& color) : color(color) {}
  ArtboardData(io::DataDecoder& decoder);
};

/**
 * @brief ArtboardComponent wrapper.
 *
 * Artboards and backgrounds are implemented through this component.
 */
struct ArtboardComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 11;  // The component id.
  using Data = ArtboardData;                   // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  ArtboardComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Returns the artboard color.
   *
   * @return The artboard color.
   */
  inline vec4 color() const
  {
    return m_data->color;
  }

  /**
   * @brief Sets the artboard color.
   *
   * @param color The new artboard color.
   */
  void color(const vec4& color);

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
