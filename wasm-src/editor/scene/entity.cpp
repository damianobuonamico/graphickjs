#include "entity.h"

#include "../../utils/console.h"

namespace Graphick::Editor {

  Entity::Entity(entt::entity handle, Scene* scene, const std::vector<uint8_t>& encoded_data) :
    m_handle(handle),
    m_scene(scene)
  {
    size_t index = 0;
    while (index < encoded_data.size()) {
      uint8_t component_id = encoded_data[index];
      index++;

      switch (component_id) {
      case IDComponent::component_id: {
        IDComponent id_component(encoded_data, index);
        m_scene->m_registry.emplace<IDComponent>(m_handle, id_component);
        break;
      }
      case TagComponent::component_id: {
        TagComponent tag_component(encoded_data, index);
        m_scene->m_registry.emplace<TagComponent>(m_handle, tag_component);
        break;
      }
      case PathComponent::component_id: {
        PathComponent path_component(encoded_data, index);
        m_scene->m_registry.emplace<PathComponent>(m_handle, path_component);
        break;
      }
      case TransformComponent::component_id: {
        TransformComponent transform_component(id(), encoded_data, index,
          has_component<PathComponent>() ? &m_scene->m_registry.get<PathComponent>(m_handle) : nullptr
        );
        m_scene->m_registry.emplace<TransformComponent>(m_handle, transform_component);
        break;
      }
      case StrokeComponent::component_id: {
        StrokeComponent stroke_component(encoded_data, index);
        m_scene->m_registry.emplace<StrokeComponent>(m_handle, stroke_component);
        break;
      }
      case FillComponent::component_id: {
        FillComponent fill_component(encoded_data, index);
        m_scene->m_registry.emplace<FillComponent>(m_handle, fill_component);
        break;
      }
      default: {
        console::log("Unknown component ID: ", component_id);
        break;
      }
      }
    }
  }

  std::vector<uint8_t> Entity::encode() const {
    std::vector<uint8_t> data;

    auto id_handle = m_scene->m_registry.try_get<IDComponent>(m_handle);
    auto tag_handle = m_scene->m_registry.try_get<TagComponent>(m_handle);
    auto path_handle = m_scene->m_registry.try_get<PathComponent>(m_handle);
    auto transform_handle = m_scene->m_registry.try_get<TransformComponent>(m_handle);
    auto stroke_handle = m_scene->m_registry.try_get<StrokeComponent>(m_handle);
    auto fill_handle = m_scene->m_registry.try_get<FillComponent>(m_handle);

    if (id_handle) {
      auto id_data = id_handle->encode();

      if (!id_data.empty()) {
        data.push_back(IDComponent::component_id);
        data.insert(data.end(), id_data.begin(), id_data.end());
      }
    }

    if (tag_handle) {
      auto tag_data = tag_handle->encode();

      if (!tag_data.empty()) {
        data.push_back(TagComponent::component_id);
        data.insert(data.end(), tag_data.begin(), tag_data.end());
      }
    }

    if (path_handle) {
      auto path_data = path_handle->encode();

      if (!path_data.empty()) {
        data.push_back(PathComponent::component_id);
        data.insert(data.end(), path_data.begin(), path_data.end());
      }
    }

    if (transform_handle) {
      auto transform_data = transform_handle->encode();

      if (!transform_data.empty()) {
        data.push_back(TransformComponent::component_id);
        data.insert(data.end(), transform_data.begin(), transform_data.end());
      }
    }

    if (stroke_handle) {
      auto stroke_data = stroke_handle->encode();

      if (!stroke_data.empty()) {
        data.push_back(StrokeComponent::component_id);
        data.insert(data.end(), stroke_data.begin(), stroke_data.end());
      }
    }

    if (fill_handle) {
      auto fill_data = fill_handle->encode();

      if (!fill_data.empty()) {
        data.push_back(FillComponent::component_id);
        data.insert(data.end(), fill_data.begin(), fill_data.end());
      }
    }

    // for (auto&& curr : m_scene->m_registry.storage()) {
    //   if (auto& storage = curr.second; storage.contains(m_handle)) {
    //     entt::id_type component_id = curr.first;
    //     console::log("Component ID: ", storage.get(m_handle)->encode());
    //     // data.push_back(curr.first);
    //     // std::vector<uint8_t> component_data = storage.get(m_handle).encode();
    //     // data.insert(data.end(), component_data.begin(), component_data.end());
    //   }
    // }

    // entt::meta_range<entt::

    // // IDComponent
    // data.push_back(IDComponent::component_id);
    // std::vector<uint8_t> id_data = m_scene->m_registry.get<IDComponent>(m_handle).encode();
    // data.insert(data.end(), id_data.begin(), id_data.end());

    // // TagComponent
    // if (m_scene->m_registry.all_of<TagComponent>(m_handle)) {
    //   data.push_back(TagComponent::component_id);
    //   std::vector<uint8_t> tag_data = m_scene->m_registry.get<TagComponent>(m_handle).encode();
    //   data.insert(data.end(), tag_data.begin(), tag_data.end());
    // }

    // // CategoryComponent
    // if (m_scene->m_registry.all_of<CategoryComponent>(m_handle)) {
    //   data.push_back(CategoryComponent::component_id);
    //   std::vector<uint8_t> category_data = m_scene->m_registry.get<CategoryComponent>(m_handle).encode();
    //   data.insert(data.end(), category_data.begin(), category_data.end());
    // }

    // // PathComponent
    // if (m_scene->m_registry.all_of<PathComponent>(m_handle)) {
    //   data.push_back(PathComponent::component_id);
    //   std::vector<uint8_t> path_data = m_scene->m_registry.get<PathComponent>(m_handle).encode();
    //   data.insert(data.end(), path_data.begin(), path_data.end());
    // }

    return data;
  }

}
