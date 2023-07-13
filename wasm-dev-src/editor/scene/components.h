#pragma once

#include "../../history/values/vec2_value.h"

#include "../../utils/uuid.h"

#include "../../renderer/geometry/path.h"

#include "../../lib/blaze/src/Matrix.h"

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

    Blaze::Matrix get_matrix() const {
      Blaze::Matrix m = Blaze::Matrix::CreateScale(scale.x, scale.y);
      m.PostRotate(rotation);
      m.PostTranslate(position.get().x, position.get().y);

      return m;
    }
  };

}
