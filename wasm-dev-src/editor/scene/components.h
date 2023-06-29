#pragma once

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

  struct PathComponent {
    Renderer::Geometry::Path path;

    PathComponent() = default;
    PathComponent(const PathComponent& other) = default;
    PathComponent(const Renderer::Geometry::Path& path) : path(path) {}
  };

}
