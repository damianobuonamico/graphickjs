/**
 * @file components.cpp
 * @brief This file includes the implementations of all of the components.
 *
 * @todo implement encoding diffing and generic optimization
 * @todo implement PathComponent::modify()
 * @todo avoid going through the history execute system for the first time an action is performed
 */

#include "components.h"

#include "entity.h"

#include "../editor.h"

#include "../../renderer/geometry/path.h"

#include "../../math/matrix.h"
#include "../../math/math.h"

#include "../../utils/console.h"  

namespace Graphick::Editor {

  /* -- IDComponent -- */

  IDComponentData::IDComponentData() : id({}) {}

  IDComponentData::IDComponentData(const uuid id) : id(id) {}

  IDComponentData::IDComponentData(io::DataDecoder& decoder) : id(decoder.uuid()) {}

  io::EncodedData& IDComponent::encode(io::EncodedData& data, const bool optimize) const {
    if (optimize && id() == uuid::null) return data;

    data.component_id(component_id)
      .uuid(id());

    return data;
  }

  /* -- TagComponent -- */

  TagComponentData::TagComponentData(const std::string& tag) : tag(tag) {}

  TagComponentData::TagComponentData(io::DataDecoder& decoder) : tag(decoder.string()) {}

  io::EncodedData& TagComponent::encode(io::EncodedData& data, const bool optimize) const {
    if (optimize && tag().empty()) return data;

    data.component_id(component_id)
      .string(tag());

    return data;
  }

  void TagComponent::modify(io::DataDecoder& decoder) {
    m_data->tag = decoder.string();
  }

  /* -- CategoryComponent -- */

  CategoryComponentData::CategoryComponentData(const int category) : category(category) {}

  CategoryComponentData::CategoryComponentData(io::DataDecoder& decoder) : category(static_cast<Category>(decoder.uint8())) {}

  io::EncodedData& CategoryComponent::encode(io::EncodedData& data, const bool optimize) const {
    if (optimize && category() == Category::None) return data;

    data.component_id(component_id)
      .uint8(static_cast<uint8_t>(category()));

    return data;
  }

  void CategoryComponent::modify(io::DataDecoder& decoder) {
    m_data->category = static_cast<Category>(decoder.uint8());
  }

  /* -- PathComponent -- */

  PathComponentData::PathComponentData() : path() {}

  PathComponentData::PathComponentData(const Renderer::Geometry::Path& path) : path(path) {}

  PathComponentData::PathComponentData(io::DataDecoder& decoder) : path(decoder) {}

  io::EncodedData& PathComponent::encode(io::EncodedData& data, const bool optimize) const {
    data.component_id(component_id);

    return m_data->path.encode(data);
  }

  void PathComponent::modify(io::DataDecoder& decoder) { }

  /* -- TransformComponent -- */

  TransformComponentData::TransformComponentData(const mat2x3& matrix) : matrix(matrix) {}

  TransformComponentData::TransformComponentData(io::DataDecoder& decoder) : matrix(decoder.mat2x3()) {}

  mat2x3 TransformComponent::inverse() const {
    return Math::inverse(m_data->matrix);
  }

  rect TransformComponent::bounding_rect() const {
    if (!m_path_ptr) {
      return Math::translation(m_data->matrix);
    }

    const float angle = Math::rotation(m_data->matrix);

    if (Math::is_almost_zero(std::fmodf(angle, MATH_F_TWO_PI))) {
      return m_data->matrix * m_path_ptr->path.bounding_rect();
    } else {
      return m_path_ptr->path.bounding_rect(m_data->matrix);
    }
  }

  rect TransformComponent::approx_bounding_rect() const {
    if (!m_path_ptr) {
      return Math::translation(m_data->matrix);
    }

    const rect path_rect = m_path_ptr->path.approx_bounding_rect();

    return m_data->matrix * path_rect;
  }

  vec2 TransformComponent::revert(const vec2 point) const {
    return inverse() * point;
  }

  void TransformComponent::translate(const vec2 delta) {
    GK_TOTAL("TransformComponent::translate");

    if (Math::is_almost_zero(delta)) return;

    io::EncodedData new_data;
    io::EncodedData backup_data;

    encode(backup_data);

    new_data.component_id(component_id)
      .mat2x3(Math::translate(m_data->matrix, delta));

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  void TransformComponent::scale(const vec2 delta) {
    GK_TOTAL("TransformComponent::scale");

    if (Math::is_almost_zero(delta)) return;

    io::EncodedData new_data;
    io::EncodedData backup_data;

    encode(backup_data);

    new_data.component_id(component_id)
      .mat2x3(Math::scale(m_data->matrix, delta));

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  void TransformComponent::rotate(const float angle) {
    GK_TOTAL("TransformComponent::rotate");

    if (Math::is_almost_zero(angle)) return;

    io::EncodedData new_data;
    io::EncodedData backup_data;

    encode(backup_data);

    new_data.component_id(component_id)
      .mat2x3(Math::rotate(m_data->matrix, angle));

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  void TransformComponent::set(const mat2x3 matrix) {
    GK_TOTAL("TransformComponent::set");

    if (m_data->matrix == matrix) return;

    io::EncodedData new_data;
    io::EncodedData backup_data;

    encode(backup_data);

    new_data.component_id(component_id)
      .mat2x3(matrix);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  io::EncodedData& TransformComponent::encode(io::EncodedData& data, const bool optimize) const {
    if (optimize && m_data->matrix == mat2x3(1.0f)) return data;

    data.component_id(component_id)
      .mat2x3(m_data->matrix);

    return data;
  }

  void TransformComponent::modify(io::DataDecoder& decoder) {
    m_data->matrix = decoder.mat2x3();
  }

  /* -- StrokeComponent -- */

  StrokeComponentData::StrokeComponentData(const vec4& color) : color(color) {}

  StrokeComponentData::StrokeComponentData(io::DataDecoder& decoder) : color(decoder.color()) {}

  io::EncodedData& StrokeComponent::encode(io::EncodedData& data, const bool optimize) const {
    if (optimize && m_data->color == vec4(0.0f, 0.0f, 0.0f, 1.0f)) return data;

    data.component_id(component_id)
      .color(m_data->color);

    return data;
  }

  void StrokeComponent::modify(io::DataDecoder& decoder) {
    m_data->color = decoder.color();
  }

  /* -- FillComponent -- */

  FillComponentData::FillComponentData(const vec4& color) : color(color) {}

  FillComponentData::FillComponentData(io::DataDecoder& decoder) : color(decoder.color()) {}

  io::EncodedData& FillComponent::encode(io::EncodedData& data, const bool optimize) const {
    if (optimize && m_data->color == vec4(0.0f, 0.0f, 0.0f, 1.0f)) return data;

    data.component_id(component_id)
      .color(m_data->color);

    return data;
  }

  void FillComponent::modify(io::DataDecoder& decoder) {
    m_data->color = decoder.color();
  }

}
