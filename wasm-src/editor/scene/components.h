/**
 * @file components.h
 * @brief Components to be added to entities.
 *
 * @todo doc
 */

#pragma once

#include "../../history/values.h"

#include "../../renderer/geometry/path_dev.h"
#include "../../renderer/properties.h"

#include "../../math/vec4.h"
#include "../../math/rect.h"
#include "../../math/mat2x3.h"

#include <string>

namespace Graphick::Editor {

  struct IDComponent {
    uuid id;

    IDComponent() = default;
    IDComponent(const IDComponent& other) = default;
    IDComponent(const uuid id) : id(id) {}
  };

  struct TagComponent {
    std::string tag;

    TagComponent() = default;
    TagComponent(const TagComponent& other) = default;
    TagComponent(const std::string& tag) : tag(tag) {}
  };

  struct CategoryComponent {
    enum Category {
      None = 0,
      Selectable = 1 << 0,
    };

    int category = None;
  };

  struct PathComponent {
    Renderer::Geometry::PathDev data;    /* The path data. */

    /**
     * @brief Constructors, copy constructor and move constructor.
     */
    PathComponent();
    PathComponent(const Renderer::Geometry::PathDev& data);
    PathComponent(const PathComponent& other);
    PathComponent(PathComponent&& other) noexcept;

    /**
     * @brief Default destructor.
     */
    ~PathComponent() = default;

    /**
     * @brief Copy and move assignment operators.
     */
    PathComponent& operator=(const PathComponent& other);
    PathComponent& operator=(PathComponent&& other) noexcept;
  };

  struct TransformComponent {
    History::Vec2Value position = { 0.0f, 0.0f };
    // vec2 scale = { 1.0f, 1.0f };
    // float rotation = 0.0f;

    TransformComponent(const PathComponent* path_ptr = nullptr);

    inline mat2x3 get() const { return m_matrix.get(); }
    inline mat2x3 inverse() const { return m_matrix.inverse(); }
    inline History::Mat2x3Value* _value() { return &m_matrix; }

    rect bounding_rect() const;
    rect approx_bounding_rect() const;

    inline vec2 transform(vec2 point) const { return m_matrix.get() * point; }
    inline vec2 revert(vec2 point) const { return inverse() * point; }

    inline void translate(vec2 delta) { m_matrix.translate(delta); }
    inline void scale(vec2 delta) { m_matrix.scale(delta); }
    inline void rotate(float delta) { m_matrix.rotate(delta); }

    inline void apply() { m_matrix.apply(); }
  private:
    History::Mat2x3Value m_matrix;

    const PathComponent* m_path_ptr;    /* A pointer to the path component of the entity, can be nullptr if the entity is not an element. */
  };

  struct StrokeComponent {
    History::Vec4Value color = { 0.0f, 0.0f, 0.0f, 1.0f };
    History::FloatValue width = 1.0f;
    History::EnumValue<Renderer::LineCap> cap = Renderer::LineCap::Butt;
    History::EnumValue<Renderer::LineJoin> join = Renderer::LineJoin::Miter;
    History::FloatValue miter_limit = 10.0f;
    History::BoolValue visible = true;

    StrokeComponent() = default;
    StrokeComponent(const StrokeComponent& other) = default;
    StrokeComponent(const vec4& color) : color(color) {}
  };

  struct FillComponent {
    History::Vec4Value color = { 0.0f, 0.0f, 0.0f, 1.0f };
    History::EnumValue<Renderer::FillRule> rule = Renderer::FillRule::NonZero;
    History::BoolValue visible = true;

    FillComponent() = default;
    FillComponent(const FillComponent& other) = default;
    FillComponent(const vec4& color) : color(color) {}
  };

}
