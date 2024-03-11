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

  size_t PathComponent::move_to(const vec2 p0) {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    m_data->path.move_to(p0);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    return 0;
  }

  size_t PathComponent::line_to(const vec2 p1, const bool reverse) {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    m_data->path.line_to(p1, reverse);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );

    return reverse ? 0 : (m_data->path.points_size() - 1);
  }

  size_t PathComponent::cubic_to(const vec2 p1, const vec2 p2, const vec2 p3, const bool reverse) {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    m_data->path.cubic_to(p1, p2, p3, reverse);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    return reverse ? 0 : (m_data->path.points_size() - 1);
  }

  size_t PathComponent::close() {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    m_data->path.close();

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    return m_data->path.points_size() - 1;

    // const bool has_in_handle = m_data->path.has_in_handle();
    // const bool has_out_handle = m_data->path.has_out_handle();

    // if (!has_in_handle && !has_out_handle) return line_to(m_data->path.point_at(0), true);

    // const vec2 p1 = m_data->path.point_at(Renderer::Geometry::Path::out_handle_index);
    // const vec2 p2 = m_data->path.point_at(Renderer::Geometry::Path::in_handle_index);
    // const vec2 p3 = m_data->path.point_at(0);

    // return cubic_to(p1, p2, p3, true);
  }

  void PathComponent::translate(const size_t point_index, const vec2 delta) {
    GK_TOTAL("PathComponent::translate");

    if (Math::is_almost_zero(delta)) return;

    io::EncodedData backup_data;
    io::EncodedData new_data;

    vec2 backup_position = m_data->path.point_at(point_index);
    vec2 new_position = backup_position + delta;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::ModifyPoint))
      .uint32(static_cast<uint32_t>(point_index))
      .vec2(backup_position);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::ModifyPoint))
      .uint32(static_cast<uint32_t>(point_index))
      .vec2(new_position);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  size_t PathComponent::to_line(const size_t command_index) {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    const size_t prev_points = m_data->path.points_size();

    // TODO: check if necessary to perform the operation
    m_data->path.to_line(command_index);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );

    return prev_points - m_data->path.points_size();
  }

  size_t PathComponent::to_cubic(const size_t command_index, const size_t reference_point) {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    // TODO: check if necessary to perform the operation
    const size_t index = m_data->path.to_cubic(command_index, reference_point);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );

    return index;
  }

  size_t PathComponent::split(const size_t segment_index, const float t) {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    const size_t index = m_data->path.split(segment_index, t);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );

    return index;
  }

  void PathComponent::remove(const size_t index, const bool keep_shape) {
    io::EncodedData backup_data;
    io::EncodedData new_data;

    backup_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    backup_data = m_data->path.encode(backup_data);

    const size_t prev_points = m_data->path.points_size();

    m_data->path.remove(index, keep_shape);

    new_data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));

    new_data = m_data->path.encode(new_data);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  io::EncodedData& PathComponent::encode(io::EncodedData& data, const bool optimize) const {
    data.component_id(component_id);

    return m_data->path.encode(data);
  }

  void PathComponent::modify(io::DataDecoder& decoder) {
    PathModifyType type = static_cast<PathModifyType>(decoder.uint8());

    switch (type) {
    case PathModifyType::ModifyPoint: {
      size_t point_index = decoder.uint32();
      vec2 old_position = m_data->path.point_at(point_index);
      vec2 new_position = decoder.vec2();

      m_data->path.translate(point_index, new_position - old_position);

      break;
    }
    case PathModifyType::LoadData: {
      m_data->path = Renderer::Geometry::Path(decoder);
      break;
    }
    }
  }

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
