/**
 * @file entity.cpp
 * @brief Entity class implementation
 */

#include "entity.h"

#include "../../utils/console.h"

namespace graphick::editor {

#define DECODE_COMPONENT(component_type) \
  case (component_type::component_id): { \
    add<component_type>(decoder); \
    break; \
  }

#define ENCODE_COMPONENT(component_type) \
  if (auto handle = m_scene->m_registry.try_get<component_type::Data>(m_handle); handle) { \
    component_type(this, handle).encode(data); \
  }

/* component_type::Data(decoder) moves the decoder to the next component. */
#define REMOVE_COMPONENT(component_type) \
  case (component_type::component_id): { \
    remove<component_type>(); \
    component_type::Data(decoder); \
    break; \
  }

#define MODIFY_COMPONENT(component_type) \
  case (component_type::component_id): { \
    get_component<component_type>().modify(decoder); \
    break; \
  }

Entity::Entity(entt::entity handle, Scene* scene, const io::EncodedData& encoded_data)
    : m_handle(handle), m_scene(scene)
{
  add(encoded_data, true);
}

io::EncodedData Entity::encode() const
{
  io::EncodedData data;

  ENCODE_COMPONENT(IDComponent);
  ENCODE_COMPONENT(TagComponent);
  ENCODE_COMPONENT(CategoryComponent);
  ENCODE_COMPONENT(TransformComponent);
  ENCODE_COMPONENT(PathComponent);
  ENCODE_COMPONENT(FillComponent);
  ENCODE_COMPONENT(StrokeComponent);
  ENCODE_COMPONENT(ImageComponent);
  ENCODE_COMPONENT(TextComponent);
  ENCODE_COMPONENT(GroupComponent);
  ENCODE_COMPONENT(LayerComponent);
  ENCODE_COMPONENT(ArtboardComponent);

  return data;
}

std::pair<uuid, io::EncodedData> Entity::duplicate() const
{
  io::EncodedData data;

  IDData id_data = {uuid()};
  IDComponent(this, &id_data).encode(data);

  if (auto tag_handle = m_scene->m_registry.try_get<TagComponent::Data>(m_handle); tag_handle) {
    TagData tag_data = {tag_handle->tag + " (Copy)"};
    TagComponent(this, &tag_data).encode(data);
  }

  ENCODE_COMPONENT(CategoryComponent);
  ENCODE_COMPONENT(TransformComponent);
  ENCODE_COMPONENT(PathComponent);
  ENCODE_COMPONENT(FillComponent);
  ENCODE_COMPONENT(StrokeComponent);
  ENCODE_COMPONENT(ImageComponent);
  ENCODE_COMPONENT(TextComponent);
  ENCODE_COMPONENT(GroupComponent);
  ENCODE_COMPONENT(LayerComponent);
  ENCODE_COMPONENT(ArtboardComponent);

  return {id_data.id, data};
}

void Entity::add(const io::EncodedData& encoded_data, const bool full_entity)
{
  io::DataDecoder decoder(&encoded_data);

  while (!decoder.end_of_data()) {
    const uint8_t component_id = decoder.component_id();

    switch (component_id) {
      DECODE_COMPONENT(IDComponent);
      DECODE_COMPONENT(TagComponent);
      DECODE_COMPONENT(CategoryComponent);
      DECODE_COMPONENT(TransformComponent);
      DECODE_COMPONENT(PathComponent);
      DECODE_COMPONENT(FillComponent);
      DECODE_COMPONENT(StrokeComponent);
      DECODE_COMPONENT(ImageComponent);
      DECODE_COMPONENT(TextComponent);
      DECODE_COMPONENT(GroupComponent);
      DECODE_COMPONENT(LayerComponent);
      DECODE_COMPONENT(ArtboardComponent);
    }
  }
}

void Entity::remove(const io::EncodedData& encoded_data)
{
  io::DataDecoder decoder(&encoded_data);

  while (!decoder.end_of_data()) {
    const uint8_t component_id = decoder.component_id();

    switch (component_id) {
      REMOVE_COMPONENT(TagComponent);
      REMOVE_COMPONENT(CategoryComponent);
      REMOVE_COMPONENT(TransformComponent);
      REMOVE_COMPONENT(PathComponent);
      REMOVE_COMPONENT(FillComponent);
      REMOVE_COMPONENT(StrokeComponent);
      REMOVE_COMPONENT(ImageComponent);
      REMOVE_COMPONENT(TextComponent);
      REMOVE_COMPONENT(GroupComponent);
      REMOVE_COMPONENT(LayerComponent);
      REMOVE_COMPONENT(ArtboardComponent);
    }
  }
}

void Entity::modify(const io::EncodedData& encoded_data)
{
  io::DataDecoder decoder(&encoded_data);

  while (!decoder.end_of_data()) {
    const uint8_t component_id = decoder.component_id();

    switch (component_id) {
      MODIFY_COMPONENT(TagComponent);
      MODIFY_COMPONENT(CategoryComponent);
      MODIFY_COMPONENT(TransformComponent);
      MODIFY_COMPONENT(PathComponent);
      MODIFY_COMPONENT(FillComponent);
      MODIFY_COMPONENT(StrokeComponent);
      MODIFY_COMPONENT(ImageComponent);
      MODIFY_COMPONENT(TextComponent);
      MODIFY_COMPONENT(GroupComponent);
      MODIFY_COMPONENT(LayerComponent);
      MODIFY_COMPONENT(ArtboardComponent);
    }
  }
}

}  // namespace graphick::editor
