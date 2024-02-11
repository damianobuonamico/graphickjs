/**
 * @file entity.cpp
 * @brief Entity class implementation
 */

#include "entity.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

#define DECODE_COMPONENT(component_type) if (component_id == component_type::component_id) { \
  add<component_type>(decoder); \
  if (decoder.end_of_data()) return; \
  component_id = decoder.component_id(); }

#define DECODE_COMPONENT_WITH_DATA(component_type, ...) if (component_id == component_type::component_id) { \
  add<component_type>(__VA_ARGS__); \
  if (decoder.end_of_data()) return; \
  component_id = decoder.component_id(); }

#define ENCODE_COMPONENT(component_type) if (auto handle = m_scene->m_registry.try_get<component_type>(m_handle); handle) { \
  handle->encode(data); }

#define REMOVE_COMPONENT(component_type) if (component_id == component_type::component_id) { \
  remove<component_type>(); }

  Entity::Entity(entt::entity handle, Scene* scene, const io::EncodedData& encoded_data) :
    m_handle(handle),
    m_scene(scene)
  {
    add(encoded_data);
  }

  io::EncodedData Entity::encode() const {
    io::EncodedData data;

    ENCODE_COMPONENT(IDComponent);
    ENCODE_COMPONENT(TagComponent);
    ENCODE_COMPONENT(CategoryComponent);
    ENCODE_COMPONENT(PathComponent);
    ENCODE_COMPONENT(TransformComponent);
    ENCODE_COMPONENT(StrokeComponent);
    ENCODE_COMPONENT(FillComponent);

    return data;
  }

  void Entity::add(const io::EncodedData& encoded_data) {
    io::DataDecoder decoder(&encoded_data);

    if (decoder.end_of_data()) return;

    uint8_t component_id = decoder.component_id();

    DECODE_COMPONENT(IDComponent);
    DECODE_COMPONENT(TagComponent);
    DECODE_COMPONENT(CategoryComponent);
    DECODE_COMPONENT(PathComponent);
    DECODE_COMPONENT_WITH_DATA(TransformComponent, id(), decoder, has_component<PathComponent>() ? &get_component<PathComponent>() : nullptr);
    DECODE_COMPONENT(StrokeComponent);
    DECODE_COMPONENT(FillComponent);
  }

  void Entity::remove(const io::EncodedData& encoded_data) {
    io::DataDecoder decoder(&encoded_data);

    if (decoder.end_of_data()) return;

    uint8_t component_id = decoder.component_id();

    REMOVE_COMPONENT(IDComponent);
    REMOVE_COMPONENT(TagComponent);
    REMOVE_COMPONENT(CategoryComponent);
    REMOVE_COMPONENT(PathComponent);
    REMOVE_COMPONENT(TransformComponent);
    REMOVE_COMPONENT(StrokeComponent);
    REMOVE_COMPONENT(FillComponent);
  }

}
