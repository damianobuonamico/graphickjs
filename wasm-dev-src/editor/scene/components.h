#pragma once

#include "../../history/values.h"

#include "../../utils/uuid.h"

#include "../../renderer/geometry/path.h"

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
    Renderer::Geometry::Path path;

    PathComponent() = default;
    PathComponent(const PathComponent& other) = default;
    PathComponent(const Renderer::Geometry::Path& path) : path(path) {}
  };

  struct TransformComponent {
    History::Vec2Value position = { 0.0f, 0.0f };
    vec2 scale = { 1.0f, 1.0f };
    float rotation = 0.0f;
  };

  struct FillComponent {
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

    FillComponent() = default;
    FillComponent(const FillComponent& other) = default;
    FillComponent(const vec4& color) : color(color) {}
  };

}
