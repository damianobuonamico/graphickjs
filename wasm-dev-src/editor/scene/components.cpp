#include "components.h"

#include "../../renderer/geometry/path.h"

namespace Graphick::Editor {

  /* -- TransformComponent -- */

  TransformComponent::TransformComponent(const PathComponent* path_ptr)
    : m_path_ptr(path_ptr) {}

  rect TransformComponent::bounding_rect() const {
    if (!m_path_ptr) return { position.get(), position.get() };

    // TODO: transform rect
    return m_path_ptr->path.bounding_rect();
  }

}