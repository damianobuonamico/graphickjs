#include "components.h"

#include "../editor.h"

#include "../../renderer/geometry/path.h"

#include "../../math/matrix.h"
#include "../../math/math.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  /* -- IDComponent -- */

  IDComponent::IDComponent(io::DataDecoder& decoder) {
    id = decoder.uuid();
  }

  io::EncodedData& IDComponent::encode(io::EncodedData& data) const {
    if (id == uuid::null) return data;

    data.component_id(component_id)
      .uuid(id);

    return data;
  }

  /* -- TagComponent -- */

  TagComponent::TagComponent(io::DataDecoder& decoder) {
    tag = decoder.string();
  }

  io::EncodedData& TagComponent::encode(io::EncodedData& data) const {
    if (tag.empty()) return data;

    data.component_id(component_id)
      .string(tag);

    return data;
  }

  /* -- CategoryComponent -- */

  CategoryComponent::CategoryComponent(io::DataDecoder& decoder) {
    category = static_cast<Category>(decoder.uint8());
  }

  io::EncodedData& CategoryComponent::encode(io::EncodedData& data) const {
    if (category == Category::None) return data;

    data.component_id(component_id)
      .uint8(static_cast<uint8_t>(category));

    return data;
  }

  /* -- PathComponent -- */

  PathComponent::PathComponent() : data() {}

  PathComponent::PathComponent(const Renderer::Geometry::Path& data) : data(data) {}

  PathComponent::PathComponent(const PathComponent& other) : data(other.data) {}

  PathComponent::PathComponent(PathComponent&& other) noexcept : data(std::move(other.data)) {}

  PathComponent::PathComponent(io::DataDecoder& decoder) : data(decoder) {}

  PathComponent& PathComponent::operator=(const PathComponent& other) {
    data = other.data;
    return *this;
  }

  PathComponent& PathComponent::operator=(PathComponent&& other) noexcept {
    data = std::move(other.data);
    return *this;
  }

  io::EncodedData& PathComponent::encode(io::EncodedData& data) const {
    data.component_id(component_id);

    return this->data.encode(data);
  }

  /* -- TransformComponent -- */

  TransformComponent::TransformComponent(const uuid entity_id, const PathComponent* path_ptr) :
    m_entity_id(entity_id),
    m_path_ptr(path_ptr) {}

  TransformComponent::TransformComponent(const uuid entity_id, io::DataDecoder& decoder, const PathComponent* path_ptr) :
    m_entity_id(entity_id),
    m_path_ptr(path_ptr)
  {
    m_matrix = decoder.mat2x3();
  }

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
    // History::History::add(
    //   History::Action::Type::Modify,
    //   m_entity_id,
    //   History::Action::Property::Transform,
    //   Math::translate(m_matrix, delta),
    //   _value()
    // );

    // Editor::scene().history.modify(
    //   m_entity_id,
    //   Action::Property::Transform,
    //   Math::translate(m_matrix, delta),
    //   _value()
    // );
  }

  void TransformComponent::scale(const vec2 delta) {
    // History::History::add(
    //   History::Action::Type::Modify,
    //   m_entity_id,
    //   History::Action::Property::Transform,
    //   Math::scale(m_matrix, delta),
    //   _value()
    // );
  }

  void TransformComponent::rotate(const float angle) {
    // History::History::add(
    //   History::Action::Type::Modify,
    //   m_entity_id,
    //   History::Action::Property::Transform,
    //   Math::rotate(m_matrix, angle),
    //   _value()
    // );
  }

  io::EncodedData& TransformComponent::encode(io::EncodedData& data) const {
    if (m_matrix == mat2x3(1.0f)) return data;

    data.component_id(component_id)
      .mat2x3(m_matrix);

    return data;
  }

  /* -- StrokeComponent -- */

  StrokeComponent::StrokeComponent(io::DataDecoder& decoder) {
    color = decoder.color();
  }

  io::EncodedData& StrokeComponent::encode(io::EncodedData& data) const {
    // if (color.get() == vec4(0.0f, 0.0f, 0.0f, 1.0f)) return data;

    data.component_id(component_id)
      .color(color.get());

    return data;
  }

  /* -- FillComponent -- */

  FillComponent::FillComponent(io::DataDecoder& decoder) {
    color = decoder.color();
  }

  io::EncodedData& FillComponent::encode(io::EncodedData& data) const {
    // if (color.get() == vec4(0.0f, 0.0f, 0.0f, 1.0f)) return data;

    data.component_id(component_id)
      .color(color.get());

    return data;
  }

}