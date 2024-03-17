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

#define ENCODE_COMPONENT(component_type) if (auto handle = m_scene->m_registry.try_get<component_type::Data>(m_handle); handle) { \
  component_type(this, handle).encode(data); }

#define REMOVE_COMPONENT(component_type) if (component_id == component_type::component_id) { \
  remove<component_type>(); }

#define MODIFY_COMPONENT(component_type) case (component_type::component_id): { \
  get_component<component_type>().modify(decoder); \
  break; }

  Entity::Entity(entt::entity handle, Scene* scene, const io::EncodedData& encoded_data) :
    m_handle(handle),
    m_scene(scene)
  {
    add(encoded_data, true);
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

  std::pair<uuid, io::EncodedData> Entity::duplicate() const {
    io::EncodedData data;

    IDComponentData id_data = { uuid() };
    IDComponent(this, &id_data).encode(data);

    if (auto tag_handle = m_scene->m_registry.try_get<TagComponent::Data>(m_handle); tag_handle) {
      TagComponentData tag_data = { tag_handle->tag + " (Copy)" };
      TagComponent(this, &tag_data).encode(data);
    }

    ENCODE_COMPONENT(CategoryComponent);
    ENCODE_COMPONENT(PathComponent);
    ENCODE_COMPONENT(TransformComponent);
    ENCODE_COMPONENT(StrokeComponent);
    ENCODE_COMPONENT(FillComponent);

    return { id_data.id, data };
  }

  void Entity::add(const io::EncodedData& encoded_data, const bool full_entity) {
    io::DataDecoder decoder(&encoded_data);

    if (decoder.end_of_data()) return;

    uint8_t component_id = decoder.component_id();

    DECODE_COMPONENT(IDComponent);
    DECODE_COMPONENT(TagComponent);
    DECODE_COMPONENT(CategoryComponent);
    DECODE_COMPONENT(PathComponent);
    DECODE_COMPONENT(TransformComponent);
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

  void Entity::modify(const io::EncodedData& encoded_data) {
    io::DataDecoder decoder(&encoded_data);
    if (decoder.end_of_data()) return;

    uint8_t component_id = decoder.component_id();
    if (decoder.end_of_data()) return;

    switch (component_id) {
      MODIFY_COMPONENT(TagComponent);
      MODIFY_COMPONENT(CategoryComponent);
      MODIFY_COMPONENT(PathComponent);
      MODIFY_COMPONENT(TransformComponent);
      MODIFY_COMPONENT(StrokeComponent);
      MODIFY_COMPONENT(FillComponent);
    }
  }

}
