#pragma once

#include "../../history/values.h"

#include "../../renderer/geometry/path.h"

#include "../../math/vec4.h"
#include "../../math/rect.h"
#include "../../math/mat2x3.h"

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
    // TODO: make shared_ptr
    Renderer::Geometry::Path path;

    PathComponent(const uuid id) : path(id) {}
    PathComponent(const uuid id, const Renderer::Geometry::Path& path) : path(id, path) {}
    PathComponent(const PathComponent& other) = default;
  };

  struct TransformComponent {
    History::Vec2Value position = { 0.0f, 0.0f };
    // vec2 scale = { 1.0f, 1.0f };
    // float rotation = 0.0f;

    TransformComponent(const PathComponent* path_ptr = nullptr);

    inline mat2x3 get() const { return m_matrix.get(); }

    rect bounding_rect() const;
    rect large_bounding_rect() const;

    inline vec2 transform(vec2 point) const { return m_matrix.get() * point; }
    inline vec2 revert(vec2 point) const { return m_matrix.get() / point; }

    inline void translate(vec2 delta) { m_matrix.translate(delta); }
    inline void scale(vec2 delta) { m_matrix.scale(delta); }
    inline void rotate(float delta) { m_matrix.rotate(delta); }

    inline void apply() { m_matrix.apply(); }
  private:
    History::Mat2x3Value m_matrix;

    const PathComponent* m_path_ptr;    /* A pointer to the path component of the entity, can be nullptr if the entity is not an element. */
  };

  struct FillComponent {
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

    FillComponent() = default;
    FillComponent(const FillComponent& other) = default;
    FillComponent(const vec4& color) : color(color) {}
  };

}
