#include "components.h"

#include "../../renderer/geometry/path.h"

#include "../../math/matrix.h"
#include "../../math/math.h"

#include "../../history/history.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  /* -- PathComponent -- */

  PathComponent::PathComponent() : data() {}

  PathComponent::PathComponent(const Renderer::Geometry::Path& data) : data(data) {}

  PathComponent::PathComponent(const PathComponent& other) : data(other.data) {}

  PathComponent::PathComponent(PathComponent&& other) noexcept : data(std::move(other.data)) {}

  PathComponent& PathComponent::operator=(const PathComponent& other) {
    data = other.data;
    return *this;
  }

  PathComponent& PathComponent::operator=(PathComponent&& other) noexcept {
    data = std::move(other.data);
    return *this;
  }

  /* -- TransformComponent -- */

  TransformComponent::TransformComponent(const uuid entity_id, const PathComponent* path_ptr) :
    m_entity_id(entity_id),
    m_path_ptr(path_ptr) {}

  mat2x3 TransformComponent::inverse() const {
    return Math::inverse(m_matrix);
  }

  rect TransformComponent::bounding_rect() const {
    if (!m_path_ptr) return { };
    // if (!m_path_ptr) return { position.get(), position.get() };

    mat2x3 matrix = get();
    float angle = Math::rotation(matrix);

    if (Math::is_almost_zero(std::fmodf(angle, MATH_F_TWO_PI))) {
      return matrix * m_path_ptr->data.bounding_rect();
    } else {
      return m_path_ptr->data.bounding_rect(matrix);
    }
  }

  rect TransformComponent::approx_bounding_rect() const {
    if (!m_path_ptr) return { };
    // if (!m_path_ptr) return { position.get(), position.get() };

    rect path_rect = m_path_ptr->data.approx_bounding_rect();
    mat2x3 matrix = get();

    return matrix * path_rect;
  }

  vec2 TransformComponent::revert(const vec2 point) const {
    return inverse() * point;
  }

  void TransformComponent::translate(const vec2 delta) {
    History::History::add(
      History::Action::Type::Modify,
      m_entity_id,
      History::Action::Property::Transform,
      Math::translate(m_matrix, delta),
      _value()
    );
  }

  void TransformComponent::scale(const vec2 delta) {
    History::History::add(
      History::Action::Type::Modify,
      m_entity_id,
      History::Action::Property::Transform,
      Math::scale(m_matrix, delta),
      _value()
    );
  }

  void TransformComponent::rotate(const float angle) {
    History::History::add(
      History::Action::Type::Modify,
      m_entity_id,
      History::Action::Property::Transform,
      Math::rotate(m_matrix, angle),
      _value()
    );
  }

}