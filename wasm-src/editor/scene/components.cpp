/**
 * @file components.cpp
 * @brief This file includes the implementations of all of the components.
 *
 * @todo implement encoding diffing and generic optimization (especially for the path component).
 * @todo translate multiple points in one history action
 */

#include "components.h"

#include "entity.h"

#include "../editor.h"

#include "../../math/matrix.h"
#include "../../math/math.h"

#include "../../geom/path.h"

#include "../../utils/console.h"  

namespace graphick::editor {

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

  size_t PathComponent::move_to(const vec2 p0) {
    commit_load([&]() {
      m_data->move_to(p0);
      return 0;
    });

    return 0;
  }

  size_t PathComponent::line_to(const vec2 p1, const bool reverse) {
    commit_load([&]() {
      m_data->line_to(p1, reverse);
      return 0;
    });

    return reverse ? 0 : (m_data->points_size() - 1);
  }

  size_t PathComponent::quadratic_to(const vec2 p1, const vec2 p2, const bool reverse) {
    commit_load([&]() {
      m_data->quadratic_to(p1, p2, reverse);
      return 0;
    });

    return reverse ? 0 : (m_data->points_size() - 1);
  }

  size_t PathComponent::cubic_to(const vec2 p1, const vec2 p2, const vec2 p3, const bool reverse) {
    commit_load([&]() {
      m_data->cubic_to(p1, p2, p3, reverse);
      return 0;
    });

    return reverse ? 0 : (m_data->points_size() - 1);
  }

  size_t PathComponent::close(const bool reverse) {
    commit_load([&]() {
      m_data->close();
      return 0;
    });

    if (reverse) {
      return std::min(
        m_data->points_size() - 1,
        m_data->points_size() - static_cast<size_t>(m_data->back().type) - 1
      );
    }

    return m_data->points_size() - 1;
  }

  void PathComponent::translate(const size_t point_index, const vec2 delta) {
    GK_TOTAL("PathComponent::translate");

    if (math::is_almost_zero(delta)) return;

    io::EncodedData backup, data;

    vec2 backup_position = m_data->point_at(point_index);
    vec2 position = backup_position + delta;

    m_data->translate(point_index, delta);

    backup.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::ModifyPoint))
      .uint32(static_cast<uint32_t>(point_index))
      .vec2(backup_position);

    data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::ModifyPoint))
      .uint32(static_cast<uint32_t>(point_index))
      .vec2(position);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(data),
      std::move(backup),
      false
    );
  }

  size_t PathComponent::to_line(const size_t command_index, const size_t reference_point) {
    if (m_data->command_at(command_index) == Data::Command::Cubic) {
      return reference_point;
    }

    return commit_load([&]() {
      return m_data->to_line(command_index, reference_point);
    });
  }

  size_t PathComponent::to_cubic(const size_t command_index, const size_t reference_point) {
    if (m_data->command_at(command_index) == Data::Command::Cubic) {
      return reference_point;
    }

    return commit_load([&]() {
      return m_data->to_cubic(command_index, reference_point);
    });
  }

  size_t PathComponent::split(const size_t segment_index, const float t) {
    return commit_load([&]() {
      return m_data->split(segment_index, t);
    });
  }

  void PathComponent::remove(const size_t index, const bool keep_shape) {
    commit_load([&]() {
      m_data->remove(index, keep_shape);
      return 0;
    });
  }

  io::EncodedData& PathComponent::encode(io::EncodedData& data, const bool optimize) const {
    data.component_id(component_id);

    return m_data->encode(data);
  }

  void PathComponent::modify(io::DataDecoder& decoder) {
    PathModifyType type = static_cast<PathModifyType>(decoder.uint8());

    switch (type) {
    case PathModifyType::ModifyPoint: {
      size_t point_index = decoder.uint32();
      vec2 old_position = m_data->point_at(point_index);
      vec2 new_position = decoder.vec2();

      m_data->translate(point_index, new_position - old_position);

      break;
    }
    case PathModifyType::LoadData: {
      *m_data = decoder;
      break;
    }
    }
  }

  size_t PathComponent::commit_load(const std::function<size_t()> action) {
    io::EncodedData backup, data;

    backup.component_id(PathComponent::component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));
    m_data->encode(backup);

    size_t index = action();

    data.component_id(PathComponent::component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));
    m_data->encode(data);

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(data),
      std::move(backup),
      false
    );

    return index;
  }

  /* -- TransformComponent -- */

  TransformComponentData::TransformComponentData(const mat2x3& matrix) : matrix(matrix) {}

  TransformComponentData::TransformComponentData(io::DataDecoder& decoder) : matrix(decoder.mat2x3()) {}

  mat2x3 TransformComponent::inverse() const {
    return math::inverse(m_data->matrix);
  }

  rect TransformComponent::bounding_rect() const {
    if (!m_path_ptr) {
      return math::translation(m_data->matrix);
    }

    const float angle = math::rotation(m_data->matrix);

    if (math::is_almost_zero(std::fmodf(angle, math::two_pi<float>))) {
      return m_data->matrix * m_path_ptr->bounding_rect();
    } else {
      return m_path_ptr->bounding_rect(m_data->matrix);
    }
  }

  rect TransformComponent::approx_bounding_rect() const {
    if (!m_path_ptr) {
      return math::translation(m_data->matrix);
    }

    const rect path_rect = m_path_ptr->approx_bounding_rect();

    return m_data->matrix * path_rect;
  }

  vec2 TransformComponent::revert(const vec2 point) const {
    return inverse() * point;
  }

  void TransformComponent::translate(const vec2 delta) {
    GK_TOTAL("TransformComponent::translate");

    if (math::is_almost_zero(delta)) return;

    io::EncodedData new_data;
    io::EncodedData backup_data;

    encode(backup_data);

    new_data.component_id(component_id)
      .mat2x3(math::translate(m_data->matrix, delta));

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  void TransformComponent::scale(const vec2 delta) {
    GK_TOTAL("TransformComponent::scale");

    if (math::is_almost_zero(delta)) return;

    io::EncodedData new_data;
    io::EncodedData backup_data;

    encode(backup_data);

    new_data.component_id(component_id)
      .mat2x3(math::scale(m_data->matrix, delta));

    m_entity->scene()->history.modify(
      m_entity->id(),
      std::move(new_data),
      std::move(backup_data)
    );
  }

  void TransformComponent::rotate(const float angle) {
    GK_TOTAL("TransformComponent::rotate");

    if (math::is_almost_zero(angle)) return;

    io::EncodedData new_data;
    io::EncodedData backup_data;

    encode(backup_data);

    new_data.component_id(component_id)
      .mat2x3(math::rotate(m_data->matrix, angle));

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
