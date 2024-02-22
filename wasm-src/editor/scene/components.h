/**
 * @file components.h
 * @brief Components to be added to entities.
 *
 * @todo doc
 */

#pragma once

#include "../../history/values.h"

#include "../../renderer/geometry/path.h"
#include "../../renderer/properties.h"

#include "../../math/vec4.h"
#include "../../math/rect.h"
#include "../../math/mat2x3.h"

#include <vector>
#include <string>

namespace Graphick::Editor {

  class Entity;

  struct IDComponent {
    static constexpr uint8_t component_id = 0;

    uuid id;

    IDComponent() = default;
    IDComponent(const IDComponent& other) = default;
    IDComponent(const uuid id) : id(id) {}
    IDComponent(io::DataDecoder& decoder);

    io::EncodedData& encode(io::EncodedData& data) const;
  };

  struct TagComponent {
    static constexpr uint8_t component_id = 1;

    std::string tag;

    TagComponent() = default;
    TagComponent(const TagComponent& other) = default;
    TagComponent(const std::string& tag) : tag(tag) {}
    TagComponent(io::DataDecoder& decoder);

    io::EncodedData& encode(io::EncodedData& data) const;
  };

  struct CategoryComponent {
    static constexpr uint8_t component_id = 2;

    enum Category {
      None = 0,
      Selectable = 1 << 0,
    };

    int category = None;

    CategoryComponent() = default;
    CategoryComponent(const CategoryComponent& other) = default;
    CategoryComponent(const int category) : category(category) {}
    CategoryComponent(io::DataDecoder& decoder);

    io::EncodedData& encode(io::EncodedData& data) const;
  };

  struct PathComponent {
    static constexpr uint8_t component_id = 3;

    Renderer::Geometry::Path data;    /* The path data. */

    /**
     * @brief Constructors, copy constructor and move constructor.
     */
    PathComponent();
    PathComponent(const Renderer::Geometry::Path& data);
    PathComponent(const PathComponent& other);
    PathComponent(PathComponent&& other) noexcept;
    PathComponent(io::DataDecoder& decoder);

    /**
     * @brief Default destructor.
     */
    ~PathComponent() = default;

    /**
     * @brief Copy and move assignment operators.
     */
    PathComponent& operator=(const PathComponent& other);
    PathComponent& operator=(PathComponent&& other) noexcept;

    io::EncodedData& encode(io::EncodedData& data) const;
  };

  /**
   * @brief Transform component used to store the transformation matrix of an entity and all its related operations.
   *
   * @struct TransformComponent
  */
  struct TransformComponent {
    static constexpr uint8_t component_id = 4;

    TransformComponent(const uuid entity_id, const PathComponent* path_ptr = nullptr);
    TransformComponent(const uuid entity_id, io::DataDecoder& decoder, const PathComponent* path_ptr = nullptr);

    /**
     * @brief Returns the transformation matrix.
     *
     * @return The transformation matrix.
     */
    inline mat2x3 get() const { return m_matrix; }

    /**
     * @brief Returns the inverse of the transformation matrix.
     *
     * @return The inverse of the transformation matrix.
     */
    mat2x3 inverse() const;

    /**
     * @brief Returns a pointer to the underlying data of the transformation matrix.
     *
     * @return A pointer to the transformation matrix data.
     */
    inline mat2x3* _value() { return &m_matrix; }

    /**
     * @brief Calculates the bounding rectangle of the entity.
     *
     * @return The bounding rectangle of the entity.
     */
    rect bounding_rect() const;

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
    inline vec2 transform(vec2 point) const { return m_matrix * point; }

    /**
     * @brief Reverts a point using the inverse of the transformation matrix.
     *
     * @param point The point to revert.
     * @return The reverted point.
     */
    vec2 revert(vec2 point) const;

    /**
     * @brief Translates the entity by a given delta.
     *
     * @param delta The translation delta.
     */
    void translate(vec2 delta);

    /**
     * @brief Scales the entity by a given delta.
     *
     * @param delta The scale delta.
     */
    inline void scale(vec2 delta);

    /**
     * @brief Rotates the entity by a given delta.
     *
     * @param delta The rotation delta.
     */
    inline void rotate(float delta);

    /**
     * @todo decide if necessary
     */
    inline void apply() { /*m_matrix.apply();*/ }

    io::EncodedData& encode(io::EncodedData& data) const;
  private:
    /**
     * @brief Modifies the transformation matrix using the given data.
     *
     * This method should only be called through the history manager.
     *
     * @param decoder The data to modify the transformation matrix with.
     */
    void modify(io::DataDecoder& decoder);
  private:
    mat2x3 m_matrix;

    const PathComponent* m_path_ptr;    /* A pointer to the path component of the entity, can be nullptr if the entity is not an element. */
    const uuid m_entity_id;             /* The id of the entity this component belongs to, it is used for history tracking. */
  private:
    friend class Entity;
  };

  struct StrokeComponent {
    static constexpr uint8_t component_id = 5;

    Graphick::History::Vec4Value color = { 0.0f, 0.0f, 0.0f, 1.0f };
    Graphick::History::FloatValue width = 1.0f;
    Graphick::History::EnumValue<Renderer::LineCap> cap = Renderer::LineCap::Butt;
    Graphick::History::EnumValue<Renderer::LineJoin> join = Renderer::LineJoin::Miter;
    Graphick::History::FloatValue miter_limit = 10.0f;
    Graphick::History::BoolValue visible = true;

    StrokeComponent() = default;
    StrokeComponent(const StrokeComponent& other) = default;
    StrokeComponent(const vec4& color) : color(color) {}
    StrokeComponent(io::DataDecoder& decoder);

    io::EncodedData& encode(io::EncodedData& data) const;
  };

  struct FillComponent {
    static constexpr uint8_t component_id = 6;

    Graphick::History::Vec4Value color = { 0.0f, 0.0f, 0.0f, 1.0f };
    Graphick::History::EnumValue<Renderer::FillRule> rule = Renderer::FillRule::NonZero;
    Graphick::History::BoolValue visible = true;

    FillComponent() = default;
    FillComponent(const FillComponent& other) = default;
    FillComponent(const vec4& color) : color(color) {}
    FillComponent(io::DataDecoder& decoder);

    io::EncodedData& encode(io::EncodedData& data) const;
  };

}
