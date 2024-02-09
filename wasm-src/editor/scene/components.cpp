#include "components.h"

#include "../editor.h"

#include "../../renderer/geometry/path.h"

#include "../../math/matrix.h"
#include "../../math/math.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  /* -- IDComponent -- */

  IDComponent::IDComponent(const std::vector<uint8_t>& encoded_data, size_t& index) {
    std::memcpy(&id, encoded_data.data() + index, sizeof(uuid));
    index += sizeof(uuid);
  }

  std::array<uint8_t, sizeof(uuid)> IDComponent::encode() {
    std::array<uint8_t, sizeof(uuid)> data;
    std::memcpy(data.data(), &id, sizeof(uuid));
    return data;
  }

  /* -- TagComponent -- */

  TagComponent::TagComponent(const std::vector<uint8_t>& encoded_data, size_t& index) {
    size_t tag_length = encoded_data[index];
    index++;
    tag = std::string(encoded_data.begin() + index, encoded_data.begin() + index + tag_length);
    index += tag_length;
  }

  std::vector<uint8_t> TagComponent::encode() {
    std::vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>(tag.size()));
    data.insert(data.end(), tag.begin(), tag.end());
    return data;
  }

  /* -- PathComponent -- */

  PathComponent::PathComponent() : data() {}

  PathComponent::PathComponent(const Renderer::Geometry::Path& data) : data(data) {}

  PathComponent::PathComponent(const PathComponent& other) : data(other.data) {}

  PathComponent::PathComponent(PathComponent&& other) noexcept : data(std::move(other.data)) {}

  PathComponent::PathComponent(const std::vector<uint8_t>& encoded_data, size_t& index) : data(encoded_data, index) {}

  PathComponent& PathComponent::operator=(const PathComponent& other) {
    data = other.data;
    return *this;
  }

  PathComponent& PathComponent::operator=(PathComponent&& other) noexcept {
    data = std::move(other.data);
    return *this;
  }

  std::vector<uint8_t> PathComponent::encode() {
    return data.encode();
  }

  /* -- TransformComponent -- */

  TransformComponent::TransformComponent(const uuid entity_id, const PathComponent* path_ptr) :
    m_entity_id(entity_id),
    m_path_ptr(path_ptr) {}

  TransformComponent::TransformComponent(const uuid entity_id, const std::vector<uint8_t>& encoded_data, size_t& index, const PathComponent* path_ptr) :
    m_entity_id(entity_id),
    m_path_ptr(path_ptr)
  {
    std::memcpy(&m_matrix, encoded_data.data() + index, sizeof(mat2x3));
    index += sizeof(mat2x3);
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
    Editor::scene().history.modify(
      m_entity_id,
      Action::Property::Transform,
      Math::translate(m_matrix, delta),
      _value()
    );
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

  std::array<uint8_t, sizeof(mat2x3)> TransformComponent::encode() {
    std::array<uint8_t, sizeof(mat2x3)> data;
    std::memcpy(data.data(), &m_matrix, sizeof(mat2x3));
    return data;
  }

  /* -- StrokeComponent -- */

  StrokeComponent::StrokeComponent(const std::vector<uint8_t>& encoded_data, size_t& index) {
    color = vec4(
      encoded_data[index] / 255.0f,
      encoded_data[index + 1] / 255.0f,
      encoded_data[index + 2] / 255.0f,
      encoded_data[index + 3] / 255.0f
    );
  }

  std::vector<uint8_t> StrokeComponent::encode() {
    std::vector<uint8_t> data;

    data.push_back(static_cast<uint8_t>(color.get().r * 255.0f));
    data.push_back(static_cast<uint8_t>(color.get().g * 255.0f));
    data.push_back(static_cast<uint8_t>(color.get().b * 255.0f));
    data.push_back(static_cast<uint8_t>(color.get().a * 255.0f));

    return data;
  }

  /* -- FillComponent -- */

  FillComponent::FillComponent(const std::vector<uint8_t>& encoded_data, size_t& index) {
    color = vec4(
      encoded_data[index] / 255.0f,
      encoded_data[index + 1] / 255.0f,
      encoded_data[index + 2] / 255.0f,
      encoded_data[index + 3] / 255.0f
    );
  }

  std::vector<uint8_t> FillComponent::encode() {
    std::vector<uint8_t> data;

    data.push_back(static_cast<uint8_t>(color.get().r * 255.0f));
    data.push_back(static_cast<uint8_t>(color.get().g * 255.0f));
    data.push_back(static_cast<uint8_t>(color.get().b * 255.0f));
    data.push_back(static_cast<uint8_t>(color.get().a * 255.0f));

    return data;
  }

}