#include "components.h"

#include "../../renderer/geometry/path.h"

#include "../../math/matrix.h"
#include "../../math/math.h"

namespace Graphick::Editor {

  /* -- TransformComponent -- */

  TransformComponent::TransformComponent(const PathComponent* path_ptr)
    : m_path_ptr(path_ptr) {}

  rect TransformComponent::bounding_rect() const {
    if (!m_path_ptr) return { position.get(), position.get() };

    mat2x3 matrix = get();
    float angle = Math::rotation(matrix);

    if (Math::is_almost_zero(std::fmodf(angle, MATH_F_TWO_PI))) {
      return matrix * m_path_ptr->path.bounding_rect();
    } else {
      return m_path_ptr->path.bounding_rect(matrix);
    }
  }

  rect TransformComponent::large_bounding_rect() const {
    if (!m_path_ptr) return { position.get(), position.get() };

    rect path_rect = m_path_ptr->path.large_bounding_rect();
    mat2x3 matrix = get();

    return matrix * path_rect;
  }

}