#pragma once

#include "../../history/values.h"

#include "../../renderer/geometry/path.h"

#include "../../math/vec4.h"
#include "../../math/rect.h"
#include "../../math/mat3.h"

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
    vec2 scale = { 1.0f, 1.0f };
    float rotation = 0.0f;

    TransformComponent(const PathComponent* path_ptr = nullptr);

    rect bounding_rect() const;
  private:
    mat3 m_matrix;

    const PathComponent* m_path_ptr;    /* A pointer to the path component of the entity, can be nullptr if the entity is not an element. */
  };

  struct FillComponent {
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

    FillComponent() = default;
    FillComponent(const FillComponent& other) = default;
    FillComponent(const vec4& color) : color(color) {}
  };

}
