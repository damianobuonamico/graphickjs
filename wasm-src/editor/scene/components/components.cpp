/**
 * @file editor/scene/components/components.cpp
 * @brief This file includes the implementations of all of the components.
 *
 * @todo implement encoding diffing and generic optimization (especially for the path component).
 * @todo translate multiple points in one history action
 * @todo refactor and implement modify for every component
 */

#include "appearance.h"
#include "base.h"
#include "group.h"

#include "../entity.h"

#include "../../editor.h"

#include "../../../io/json/json.h"
#include "../../../io/resource_manager.h"

#include "../../../math/math.h"
#include "../../../math/matrix.h"

#include "../../../utils/debugger.h"

namespace graphick::editor {

#define MODIFY_NO_EXECUTE(...) \
  io::EncodedData backup, data; \
  encode(backup); \
  __VA_ARGS__; \
  encode(data); \
  m_entity->scene()->history.modify(m_entity->id(), std::move(data), std::move(backup), false)

/* -- IDComponent -- */

IDData::IDData(io::DataDecoder& decoder) : id(decoder.uuid()) {}

io::EncodedData& IDComponent::encode(io::EncodedData& data) const
{
  return data.component_id(component_id).uuid(id());
}

/* -- TagComponent -- */

TagData::TagData(io::DataDecoder& decoder)
{
  tag = decoder.string();
}

io::EncodedData& TagComponent::encode(io::EncodedData& data) const
{
  return data.component_id(component_id).string(tag());
}

void TagComponent::modify(io::DataDecoder& decoder)
{
  *m_data = decoder;
}

/* -- CategoryComponent -- */

CategoryData::CategoryData(io::DataDecoder& decoder) : category(decoder.uint8()) {}

io::EncodedData& CategoryComponent::encode(io::EncodedData& data) const
{
  return data.component_id(component_id).uint8(category());
}

void CategoryComponent::modify(io::DataDecoder& decoder)
{
  *m_data = decoder;
}

/* -- TransformComponent -- */

TransformData::TransformData(io::DataDecoder& decoder)
{
  const auto [has_transform] = decoder.bitfield<1>();

  matrix = has_transform ? decoder.mat2x3() : mat2x3::identity();
}

mat2x3 TransformComponent::inverse() const
{
  return math::inverse(m_data->matrix);
}

rect TransformComponent::bounding_rect() const
{
  switch (m_parent_ptr.type()) {
    case ParentData::Type::Path:
      return m_parent_ptr.path_ptr()->path.bounding_rect(m_data->matrix);
    case ParentData::Type::Text:
      return m_data->matrix * m_parent_ptr.text_ptr()->bounding_rect();
    case ParentData::Type::Image:
      return m_data->matrix * m_parent_ptr.image_ptr()->bounding_rect();
    case ParentData::Type::Group:
      return m_data->matrix * m_parent_ptr.group_ptr()->bounding_rect(this->m_entity->scene());
    default:
      return math::translation(m_data->matrix);
  }
}

rect TransformComponent::bounding_rect(const mat2x3& parent_transform) const
{
  const mat2x3 matrix = parent_transform * m_data->matrix;

  switch (m_parent_ptr.type()) {
    case ParentData::Type::Path:
      return m_parent_ptr.path_ptr()->path.bounding_rect(matrix);
    case ParentData::Type::Text:
      return matrix * m_parent_ptr.text_ptr()->bounding_rect();
    case ParentData::Type::Image:
      return matrix * m_parent_ptr.image_ptr()->bounding_rect();
    case ParentData::Type::Group:
      return matrix * m_parent_ptr.group_ptr()->bounding_rect(this->m_entity->scene());
    default:
      return math::translation(matrix);
  }
}

rrect TransformComponent::bounding_rrect() const
{
  const float angle = math::rotation(m_data->matrix);

  if (math::is_almost_zero(angle)) {
    return bounding_rect();
  }

  const mat2x3 unrotated_matrix = math::rotate(m_data->matrix, -angle);

  switch (m_parent_ptr.type()) {
    case ParentData::Type::Path:
      return rrect(m_parent_ptr.path_ptr()->path.bounding_rect(unrotated_matrix), angle);
    case ParentData::Type::Text:
      return rrect(unrotated_matrix * m_parent_ptr.text_ptr()->bounding_rect(), angle);
    case ParentData::Type::Image:
      return rrect(unrotated_matrix * m_parent_ptr.image_ptr()->bounding_rect(), angle);
    case ParentData::Type::Group:
      return rrect(unrotated_matrix *
                       m_parent_ptr.group_ptr()->bounding_rect(this->m_entity->scene()),
                   angle);
    default:
      return math::translation(m_data->matrix);
  }
}

rrect TransformComponent::bounding_rrect(const mat2x3& parent_transform) const
{
  const mat2x3 matrix = parent_transform * m_data->matrix;
  const float angle = math::rotation(matrix);

  if (math::is_almost_zero(angle)) {
    return bounding_rect();
  }

  const mat2x3 unrotated_matrix = math::rotate(matrix, -angle);

  switch (m_parent_ptr.type()) {
    case ParentData::Type::Path:
      return rrect(m_parent_ptr.path_ptr()->path.bounding_rect(unrotated_matrix), angle);
    case ParentData::Type::Text:
      return rrect(unrotated_matrix * m_parent_ptr.text_ptr()->bounding_rect(), angle);
    case ParentData::Type::Image:
      return rrect(unrotated_matrix * m_parent_ptr.image_ptr()->bounding_rect(), angle);
    case ParentData::Type::Group:
      return rrect(unrotated_matrix *
                       m_parent_ptr.group_ptr()->bounding_rect(this->m_entity->scene()),
                   angle);
    default:
      return math::translation(matrix);
  }
}

rect TransformComponent::approx_bounding_rect() const
{
  if (!m_parent_ptr.is_path()) {
    return bounding_rect();
  }

  return m_data->matrix * m_parent_ptr.path_ptr()->path.approx_bounding_rect();
}

vec2 TransformComponent::revert(const vec2 point) const
{
  return inverse() * point;
}

void TransformComponent::translate(const vec2 delta)
{
  __debug_time_total();

  if (math::is_almost_zero(delta)) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->matrix = math::translate(m_data->matrix, delta));
}

void TransformComponent::scale(const vec2 delta)
{
  __debug_time_total();

  if (math::is_almost_zero(delta)) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->matrix = math::scale(m_data->matrix, delta));
}

void TransformComponent::rotate(const float angle)
{
  __debug_time_total();

  if (math::is_almost_zero(angle)) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->matrix = math::rotate(m_data->matrix, angle));
}

void TransformComponent::set(const mat2x3 matrix)
{
  __debug_time_total();

  if (m_data->matrix == matrix) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->matrix = matrix);
}

io::EncodedData& TransformComponent::encode(io::EncodedData& data) const
{
  const bool has_transform = m_data->matrix != mat2x3::identity();

  data.component_id(component_id);
  data.bitfield({has_transform});

  if (has_transform) {
    data.mat2x3(m_data->matrix);
  }

  return data;
}

void TransformComponent::modify(io::DataDecoder& decoder)
{
  *m_data = decoder;
}

/* -- PathComponent -- */

size_t PathComponent::move_to(const vec2 p0)
{
  commit_load([&]() {
    m_data->path.move_to(p0);
    return 0;
  });

  return 0;
}

size_t PathComponent::line_to(const vec2 p1, const bool reverse)
{
  commit_load([&]() {
    m_data->path.line_to(p1, reverse);
    return 0;
  });

  return reverse ? 0 : (m_data->path.points_count() - 1);
}

size_t PathComponent::quadratic_to(const vec2 p1, const vec2 p2, const bool reverse)
{
  commit_load([&]() {
    m_data->path.quadratic_to(p1, p2, reverse);
    return 0;
  });

  return reverse ? 0 : (m_data->path.points_count() - 1);
}

size_t PathComponent::cubic_to(const vec2 p1, const vec2 p2, const vec2 p3, const bool reverse)
{
  commit_load([&]() {
    m_data->path.cubic_to(p1, p2, p3, reverse);
    return 0;
  });

  return reverse ? 0 : (m_data->path.points_count() - 1);
}

size_t PathComponent::close(const bool reverse)
{
  commit_load([&]() {
    m_data->path.close();
    return 0;
  });

  if (reverse) {
    return std::min(m_data->path.points_count() - 1,
                    m_data->path.points_count() - static_cast<uint32_t>(m_data->path.back().type) -
                        1);
  }

  return m_data->path.points_count() - 1;
}

// TODO: join path modify actions
void PathComponent::translate(const size_t point_index, const vec2 delta)
{
  __debug_time_total();

  if (math::is_almost_zero(delta))
    return;

  io::EncodedData backup, data;

  vec2 backup_position = m_data->path.at(point_index);
  vec2 position = backup_position + delta;

  m_data->path.translate(point_index, delta);

  backup.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::ModifyPoint))
      .uint32(static_cast<uint32_t>(point_index))
      .vec2(backup_position);

  data.component_id(component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::ModifyPoint))
      .uint32(static_cast<uint32_t>(point_index))
      .vec2(position);

  m_entity->scene()->history.modify(m_entity->id(), std::move(data), std::move(backup), false);
}

size_t PathComponent::to_line(const size_t command_index, const size_t reference_point)
{
  if (m_data->path.command_at(command_index) == geom::path::Command::Cubic) {
    return reference_point;
  }

  return commit_load([&]() { return m_data->path.to_line(command_index, reference_point); });
}

size_t PathComponent::to_cubic(const size_t command_index, const size_t reference_point)
{
  if (m_data->path.command_at(command_index) == geom::path::Command::Cubic) {
    return reference_point;
  }

  return commit_load([&]() { return m_data->path.to_cubic(command_index, reference_point); });
}

size_t PathComponent::split(const size_t segment_index, const float t)
{
  return commit_load([&]() { return m_data->path.split(segment_index, t); });
}

void PathComponent::remove(const size_t index, const bool keep_shape)
{
  commit_load([&]() {
    m_data->path.remove(index, keep_shape);
    return 0;
  });
}

io::EncodedData& PathComponent::encode(io::EncodedData& data) const
{
  data.component_id(component_id);

  return m_data->path.encode(data);
}

void PathComponent::modify(io::DataDecoder& decoder)
{
  PathModifyType type = static_cast<PathModifyType>(decoder.uint8());

  switch (type) {
    case PathModifyType::ModifyPoint: {
      size_t point_index = decoder.uint32();
      vec2 old_position = m_data->path.at(point_index);
      vec2 new_position = decoder.vec2();

      m_data->path.translate(point_index, new_position - old_position);

      break;
    }
    case PathModifyType::LoadData: {
      *m_data = decoder;
      break;
    }
  }
}

size_t PathComponent::commit_load(const std::function<size_t()> action)
{
  io::EncodedData backup, data;

  backup.component_id(PathComponent::component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));
  m_data->path.encode(backup);

  size_t index = action();

  data.component_id(PathComponent::component_id)
      .uint8(static_cast<uint8_t>(PathModifyType::LoadData));
  m_data->path.encode(data);

  m_entity->scene()->history.modify(m_entity->id(), std::move(data), std::move(backup), false);

  return index;
}

/* -- ImageComponent -- */

ImageData::ImageData(io::DataDecoder& decoder) : image_id(decoder.uuid()) {}

rect ImageData::bounding_rect() const
{
  const vec2 size = vec2(io::ResourceManager::get_image(image_id).size);
  return rect(vec2::zero(), size);
}

const uint8_t* ImageComponent::data() const
{
  return io::ResourceManager::get_image(id()).data;
}

ivec2 ImageComponent::size() const
{
  return io::ResourceManager::get_image(id()).size;
}

uint8_t ImageComponent::channels() const
{
  return io::ResourceManager::get_image(id()).channels;
}

geom::path ImageComponent::path() const
{
  const vec2 size = vec2(this->size());

  geom::path path;

  path.move_to(vec2(0.0f, 0.0f));
  path.line_to(vec2(size.x, 0.0f));
  path.line_to(size);
  path.line_to(vec2(0.0f, size.y));
  path.close();

  return path;
}

io::EncodedData& ImageComponent::encode(io::EncodedData& data) const
{
  return data.component_id(component_id).uuid(id());
}

/* -- TextComponent -- */

TextData::TextData(io::DataDecoder& decoder)
{
  const auto [has_font_id] = decoder.bitfield<1>();

  text = decoder.string();
  font_id = has_font_id ? uuid(decoder.uuid()) : uuid::null;
}

rect TextData::bounding_rect() const
{
  return rect(vec2::zero(), vec2(100.0f, 100.0f));
}

io::EncodedData& TextComponent::encode(io::EncodedData& data) const
{
  const bool has_font_id = font_id() != uuid::null;

  data.component_id(component_id);
  data.bitfield({has_font_id});
  data.string(m_data->text);

  if (has_font_id) {
    data.uuid(m_data->font_id);
  }

  return data;
}

/* -- FillComponent -- */

FillData::FillData(io::DataDecoder& decoder)
{
  const auto [is_color, has_paint, has_rule, is_visible] = decoder.bitfield<4>();

  paint = has_paint ? (is_color ? decoder.color() : renderer::Paint(decoder)) :
                      vec4(0.0f, 0.0f, 0.0f, 1.0f);
  rule = has_rule ? static_cast<renderer::FillRule>(decoder.uint8()) : renderer::FillRule::NonZero;
  visible = is_visible;
}

void FillComponent::color(const vec4& color)
{
  if (m_data->paint.is_color() && m_data->paint.color() == color) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->paint = renderer::Paint(color));
}

void FillComponent::rule(renderer::FillRule rule)
{
  if (m_data->rule == rule) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->rule = rule);
}

void FillComponent::visible(bool visible)
{
  if (m_data->visible == visible) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->visible = visible);
}

io::EncodedData& FillComponent::encode(io::EncodedData& data) const
{
  const bool is_color = paint().is_color();
  const bool has_paint = !is_color || (paint().color() != vec4(0.0f, 0.0f, 0.0f, 1.0f));
  const bool has_rule = rule() != renderer::FillRule::NonZero;
  const bool is_visible = visible();

  data.component_id(component_id);
  data.bitfield({is_color, has_paint, has_rule, is_visible});

  if (has_paint) {
    if (is_color) {
      data.color(paint().color());
    } else {
      paint().encode(data);
    }
  }

  if (has_rule) {
    data.uint8(static_cast<uint8_t>(rule()));
  }

  return data;
}

void FillComponent::ui_data(io::json::JSON& data) const
{
  if (data.has("fill")) {
    io::json::JSON& fill = data["fill"];

    if (fill.has("color") && fill["color"].to_vec4() != paint().color()) {
      fill["color"] = "mixed";
    }

    if (fill.has("rule") && fill["rule"].to_int() != static_cast<int>(rule())) {
      fill["rule"] = "mixed";
    }

    if (fill.has("visible") && fill["visible"].to_bool() != visible()) {
      fill["visible"] = "mixed";
    }
  } else {
    io::json::JSON& fill = data["fill"] = io::json::JSON::object();

    fill["color"] = paint().color();
    fill["rule"] = static_cast<int>(rule());
    fill["visible"] = visible();
  }
}

void FillComponent::modify(io::DataDecoder& decoder)
{
  *m_data = decoder;
}

/* -- StrokeComponent -- */

StrokeData::StrokeData(io::DataDecoder& decoder)
{
  const auto [is_color, has_paint, has_cap, has_join, has_miter_limit, has_width, is_visible] =
      decoder.bitfield<7>();

  paint = has_paint ? (is_color ? decoder.color() : renderer::Paint(decoder)) :
                      vec4(0.0f, 0.0f, 0.0f, 1.0f);
  cap = has_cap ? static_cast<renderer::LineCap>(decoder.uint8()) : renderer::LineCap::Butt;
  join = has_join ? static_cast<renderer::LineJoin>(decoder.uint8()) : renderer::LineJoin::Miter;
  miter_limit = has_miter_limit ? decoder.float32() : 10.0f;
  width = has_width ? decoder.float32() : 1.0f;
  visible = is_visible;
}

void StrokeComponent::color(const vec4& color)
{
  if (m_data->paint.is_color() && m_data->paint.color() == color) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->paint = renderer::Paint(color));
}

void StrokeComponent::cap(renderer::LineCap cap)
{
  if (m_data->cap == cap) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->cap = cap);
}

void StrokeComponent::join(renderer::LineJoin join)
{
  if (m_data->join == join) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->join = join);
}

void StrokeComponent::miter_limit(float miter_limit)
{
  if (m_data->miter_limit == miter_limit) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->miter_limit = miter_limit);
}

void StrokeComponent::width(float width)
{
  if (m_data->width == width) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->width = width);
}

void StrokeComponent::visible(bool visible)
{
  if (m_data->visible == visible) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->visible = visible);
}

io::EncodedData& StrokeComponent::encode(io::EncodedData& data) const
{
  const bool is_color = paint().is_color();
  const bool has_paint = !is_color || (paint().color() != vec4(0.0f, 0.0f, 0.0f, 1.0f));
  const bool has_cap = cap() != renderer::LineCap::Butt;
  const bool has_join = join() != renderer::LineJoin::Miter;
  const bool has_miter_limit = miter_limit() != 10.0f;
  const bool has_width = width() != 1.0f;
  const bool is_visible = visible();

  data.component_id(component_id);
  data.bitfield({is_color, has_paint, has_cap, has_join, has_miter_limit, has_width, is_visible});

  if (has_paint) {
    if (is_color) {
      data.color(paint().color());
    } else {
      paint().encode(data);
    }
  }

  if (has_cap) {
    data.uint8(static_cast<uint8_t>(cap()));
  }

  if (has_join) {
    data.uint8(static_cast<uint8_t>(join()));
  }

  if (has_miter_limit) {
    data.float32(miter_limit());
  }

  if (has_width) {
    data.float32(width());
  }

  return data;
}

void StrokeComponent::ui_data(io::json::JSON& data) const
{
  if (data.has("stroke")) {
    io::json::JSON& stroke = data["stroke"];

    if (stroke.has("color") && stroke["color"].to_vec4() != paint().color()) {
      stroke["color"] = "mixed";
    }

    if (stroke.has("width") && stroke["width"].to_float() != width()) {
      stroke["width"] = "mixed";
    }

    if (stroke.has("cap") && stroke["cap"].to_int() != static_cast<int>(cap())) {
      stroke["cap"] = "mixed";
    }

    if (stroke.has("join") && stroke["join"].to_int() != static_cast<int>(join())) {
      stroke["join"] = "mixed";
    }

    if (stroke.has("miter_limit") && stroke["miter_limit"].to_float() != miter_limit()) {
      stroke["miter_limit"] = "mixed";
    }

    if (stroke.has("visible") && stroke["visible"].to_bool() != visible()) {
      stroke["visible"] = "mixed";
    }
  } else {
    io::json::JSON& stroke = data["stroke"] = io::json::JSON::object();

    stroke["color"] = paint().color();
    stroke["width"] = width();
    stroke["cap"] = static_cast<int>(cap());
    stroke["join"] = static_cast<int>(join());
    stroke["miter_limit"] = miter_limit();
    stroke["visible"] = visible();
  }
}

void StrokeComponent::modify(io::DataDecoder& decoder)
{
  *m_data = decoder;
}

/* -- GroupComponent -- */

GroupData::GroupData(io::DataDecoder& decoder)
{
  children = decoder.vector<entt::entity>();
}

rect GroupData::bounding_rect(const Scene* scene) const
{
  rect bounding_rect;

  for (const entt::entity handle : children) {
    Entity child = Entity(handle, const_cast<Scene*>(scene));

    if (child.has_component<TransformComponent>()) {
      bounding_rect = rect::from_rects(bounding_rect,
                                       child.get_component<TransformComponent>().bounding_rect());
    }
  }

  return bounding_rect;
}

io::EncodedData& GroupComponent::encode(io::EncodedData& data) const
{
  return data.component_id(component_id).vector(m_data->children);
}

void GroupComponent::modify(io::DataDecoder& decoder)
{
  // TODO: different kind of modification (add, remove, move, etc.), like scene
  *m_data = decoder;
}

/* -- LayerComponent -- */

LayerData::LayerData(io::DataDecoder& decoder)
{
  children = decoder.vector<entt::entity>();
  color = decoder.color();
}

io::EncodedData& LayerComponent::encode(io::EncodedData& data) const
{
  return data.component_id(component_id).vector(m_data->children).color(m_data->color);
}

void LayerComponent::modify(io::DataDecoder& decoder)
{
  // TODO: different kind of modification (add, remove, move, etc.), like scene
  *m_data = decoder;
}

/* -- ArtboardComponent -- */

ArtboardData::ArtboardData(io::DataDecoder& decoder) : color(decoder.color()) {}

void ArtboardComponent::color(const vec4& color)
{
  if (m_data->color == color) {
    return;
  }

  MODIFY_NO_EXECUTE(m_data->color = color);
}

io::EncodedData& ArtboardComponent::encode(io::EncodedData& data) const
{
  return data.component_id(component_id).color(m_data->color);
}

void ArtboardComponent::modify(io::DataDecoder& decoder)
{
  *m_data = decoder;
}

}  // namespace graphick::editor
