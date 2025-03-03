/**
 * @file editor/scene/components/base.h
 * @brief Contains the base components used by all entities in the editor.
 */

#pragma once

#include "image.h"
#include "path.h"
#include "text.h"

#include "../../../math/mat2x3.h"
#include "../../../math/rect.h"
#include "../../../utils/uuid.h"

#include <string>

namespace graphick::editor {

/**
 * @brief IDComponent data.
 *
 * This struct should not be used directly, use the IDComponent wrapper instead.
 */
struct IDData {
  uuid id = uuid::null;  // The id of the entity.

  IDData() = default;
  IDData(const uuid id) : id(id) {}
  IDData(io::DataDecoder& decoder);
};

/**
 * @brief IDComponent wrapper.
 *
 * Once an IDComponent is created, it cannot be modified.
 */
struct IDComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 0;  // The component id.
  using Data = IDData;                        // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  IDComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to uuid.
   */
  inline operator uuid() const
  {
    return m_data->id;
  }

  /**
   * @brief Returns the id of the entity.
   *
   * @return The id of the entity.
   */
  inline uuid id() const
  {
    return m_data->id;
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
   * @brief An IDComponent cannot be modified.
   *
   * @param decoder A diff of the modified component's data.
   */
  inline void modify(io::DataDecoder& decoder) override {};

 private:
  Data* m_data;  // The actual component data.
};

/**
 * @brief TagComponent data.
 *
 * This struct should not be used directly, use the TagComponent wrapper instead.
 */
struct TagData {
  std::string tag = "";  // The tag of the entity.

  TagData() = default;
  TagData(const std::string& tag) : tag(tag) {}
  TagData(io::DataDecoder& decoder);
};

/**
 * @brief TagComponent wrapper.
 *
 * A tag is the display name of an entity, it isn't used internally.
 */
struct TagComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 1;  // The component id.
  using Data = TagData;                       // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  TagComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to std::string&.
   */
  inline operator const std::string&() const
  {
    return m_data->tag;
  }

  /**
   * @brief Returns the tag of the entity.
   *
   * @return The tag of the entity.
   */
  inline const std::string& tag() const
  {
    return m_data->tag;
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
 * @brief CategoryComponent data.
 *
 * This struct should not be used directly, use the CategoryComponent wrapper instead.
 */
struct CategoryData {
  enum Category : uint8_t {
    None = 0,               // The entity is not in any category.
    Selectable = 1 << 0,    // The entity can be selected.
  };

  uint8_t category = None;  // A bitfield of the category flags.

  CategoryData() = default;
  CategoryData(const uint8_t category) : category(category) {}
  CategoryData(io::DataDecoder& decoder);
};

/**
 * @brief CategoryComponent wrapper.
 *
 * A category is a set of flags that define the behavior of an entity in the editor.
 */
struct CategoryComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 2;  // The component id.

  using Data = CategoryData;                  // The component underlying data type.
  using Category = Data::Category;            // The category enum.
 public:
  /**
   * @brief Constructor.
   */
  CategoryComponent(const Entity* entity, Data* data) : ComponentWrapper(entity), m_data(data) {}

  /**
   * @brief Conversion operator to uint8_t.
   */
  inline operator uint8_t() const
  {
    return m_data->category;
  }

  /**
   * @brief Returns the category flags of the entity.
   *
   * @return The category of the entity.
   */
  inline uint8_t category() const
  {
    return m_data->category;
  }

  /**
   * @brief Checks if the entity is in the specified category.
   *
   * @param category The category to check.
   * @return true if the entity is in the category, false otherwise.
   */
  inline bool is_in_category(Category category) const
  {
    return m_data->category & category;
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
 * @brief A ParentData is a pointer to one of the components that define the entity.
 *
 * Can be: PathData*, TextData*, ImageData*.
 */
struct ParentData {
 public:
  /**
   * @brief The type of the parent component.
   */
  enum class Type : uint8_t { None, Path, Text, Image };

 public:
  /**
   * @brief Constructor.
   */
  ParentData(const std::nullptr_t ptr) : m_type(Type::None), m_ptr(nullptr) {}
  ParentData(const PathData* path_ptr) : m_type(Type::Path), m_ptr(path_ptr) {}
  ParentData(const TextData* text_ptr) : m_type(Type::Text), m_ptr(text_ptr) {}
  ParentData(const ImageData* image_ptr) : m_type(Type::Image), m_ptr(image_ptr) {}

  /**
   * @brief Checks if the parent component is valid.
   *
   * @return true if the parent component is valid, false otherwise.
   */
  inline bool is_valid() const
  {
    return m_ptr != nullptr || m_type == Type::None;
  }

  /**
   * @brief Checks if the parent component is a path.
   *
   * @return true if the parent component is a path, false otherwise.
   */
  inline bool is_path() const
  {
    return is_valid() && m_type == Type::Path;
  }

  /**
   * @brief Checks if the parent component is a text.
   *
   * @return true if the parent component is a text, false otherwise.
   */
  inline bool is_text() const
  {
    return is_valid() && m_type == Type::Text;
  }

  /**
   * @brief Checks if the parent component is an image.
   *
   * @return true if the parent component is an image, false otherwise.
   */
  inline bool is_image() const
  {
    return is_valid() && m_type == Type::Image;
  }

  /**
   * @brief Returns the type of the parent component.
   */
  inline Type type() const
  {
    return m_type;
  }

  /**
   * @brief Returns the pointer to the path parent component.
   *
   * This method does not perform any type checking, type() should be called first.
   *
   * @return The pointer to the path parent component.
   */
  inline const PathData* path_ptr() const
  {
    return static_cast<const PathData*>(m_ptr);
  }

  /**
   * @brief Returns the pointer to the text parent component.
   *
   * This method does not perform any type checking, type() should be called first.
   *
   * @return The pointer to the path parent component.
   */
  inline const TextData* text_ptr() const
  {
    return static_cast<const TextData*>(m_ptr);
  }

  /**
   * @brief Returns the pointer to the image parent component.
   *
   * This method does not perform any type checking, type() should be called first.
   *
   * @return The pointer to the path parent component.
   */
  inline const ImageData* image_ptr() const
  {
    return static_cast<const ImageData*>(m_ptr);
  }

 private:
  Type m_type;        // The type of the parent component.
  const void* m_ptr;  // The pointer to the parent component.
};

/**
 * @brief TransformComponent data.
 *
 * This struct should not be used directly, use the TransformComponent wrapper instead.
 */
struct TransformData {
  mat2x3 matrix = mat2x3::identity();  // The transformation matrix.

  TransformData() = default;
  TransformData(const mat2x3& matrix) : matrix(matrix) {}
  TransformData(io::DataDecoder& decoder);
};

/**
 * @brief TransformComponent wrapper.
 *
 * A transform is a 2x3 matrix used for translating, rotating and scaling an entity.
 * The bounding rect of an entity can be accessed through this component.
 */
struct TransformComponent : public ComponentWrapper {
 public:
  static constexpr uint8_t component_id = 3;  // The component id.
  using Data = TransformData;                 // The component underlying data type.
 public:
  /**
   * @brief Constructor.
   */
  TransformComponent(const Entity* entity, Data* data, const ParentData parent_ptr = nullptr)
      : ComponentWrapper(entity), m_data(data), m_parent_ptr(parent_ptr)
  {
  }

  /**
   * @brief Conversion operator to mat2x3.
   */
  inline operator const mat2x3&() const
  {
    return m_data->matrix;
  }

  /**
   * @brief Returns the transformation matrix of the entity.
   *
   * @return A mat2x3 representing the transformation matrix of the entity.
   */
  inline const mat2x3& matrix() const
  {
    return m_data->matrix;
  }

  /**
   * @brief Returns the inverse of the transformation matrix.
   *
   * @return A mat2x3 representing the inverse of the transformation matrix.
   */
  mat2x3 inverse() const;

  /**
   * @brief Calculates the bounding rectangle of the entity.
   *
   * @return The bounding rectangle of the entity.
   */
  rect bounding_rect() const;

  /**
   * @brief Calculates the rotated bounding rectangle of the entity.
   *
   * The rotated bounding rectangle follows the rotation of
   * 
   * @return The bounding rectangle of the entity.
   */
  rrect bounding_rrect() const;

  /**
   * @brief Calculates the approximate bounding rectangle of the entity.
   *
   * This function is faster than the bounding_rect function, but the result is not as accurate.
   *
   * @return The approximate bounding rectangle of the entity.
   */
  rect approx_bounding_rect() const;

  /**
   * @brief Transforms a point using the transformation matrix.
   *
   * @param point The point to transform.
   * @return The transformed point.
   */
  inline vec2 transform(const vec2 point) const
  {
    return m_data->matrix * point;
  }

  /**
   * @brief Reverts a point using the inverse of the transformation matrix.
   *
   * @param point The point to revert.
   * @return The reverted point.
   */
  vec2 revert(const vec2 point) const;

  /**
   * @brief Translates the entity by a given delta.
   *
   * @param delta The translation delta.
   */
  void translate(const vec2 delta);

  /**
   * @brief Scales the entity by a given delta.
   *
   * @param delta The scale delta.
   */
  void scale(const vec2 delta);

  /**
   * @brief Rotates the entity by a given delta.
   *
   * @param delta The rotation delta.
   */
  void rotate(const float delta);

  /**
   * @brief Sets the transformation matrix of the entity.
   *
   * @param matrix The new transformation matrix.
   */
  void set(const mat2x3 matrix);

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
  Data* m_data;                   // The actual component data.
 private:
  const ParentData m_parent_ptr;  // A pointer to the path component of the entity, can be nullptr
 private:
  friend class Entity;
};

}  // namespace graphick::editor
