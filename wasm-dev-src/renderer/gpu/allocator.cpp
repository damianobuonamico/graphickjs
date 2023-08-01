#include "allocator.h"

#include "../../math/scalar.h"

// Everything above 16 MB is allocated exactly.
#define MAX_BUFFER_SIZE_CLASS 16u * 1024u * 1024u

// Number of milliseconds before unused memory is purged.
//
// TODO: jemalloc uses a sigmoidal decay curve here. Consider something similar.
#define DECAY_TIME 250u

// Number of milliseconds before we can reuse an object buffer.
//
// This helps avoid stalls. This is admittedly a bit of a hack.
#define REUSE_TIME 15u

namespace Graphick::Renderer::GPU::Memory {

  Allocator* Allocator::s_instance = nullptr;

  void Allocator::init() {
    if (s_instance != nullptr) {
      console::error("Allocator already initialized, call shutdown() before reinitializing!");
      return;
    }

    s_instance = new Allocator();
  }

  void Allocator::shutdown() {
    if (s_instance == nullptr) {
      console::error("Allocator already shutdown, call init() before shutting down!");
      return;
    }

    delete s_instance;
  }

  uuid Allocator::allocate_texture(const ivec2 size, const TextureFormat format, const std::string& tag) {
    TextureDescriptor descriptor = { size, format };
    size_t byte_size = descriptor.byte_size();

    for (size_t free_object_index = 0; free_object_index < get()->m_free_objects.size(); free_object_index++) {
      FreeObject& free_object = get()->m_free_objects[free_object_index];
      FreeTexture* free_texture = reinterpret_cast<FreeTexture*>(&free_object.kind);

      if (
        !free_texture ||
        free_texture->allocation.kind != AllocationKind::Texture ||
        free_texture->allocation.descriptor != descriptor
        ) continue;

      uuid id = free_texture->id;

      free_texture->allocation.tag = tag;
      get()->m_bytes_committed += free_texture->allocation.descriptor.byte_size();
      get()->m_textures_in_use.insert({ id, free_texture->allocation });
      get()->m_free_objects.erase(get()->m_free_objects.begin() + free_object_index);

      return id;
    }

    TextureAllocation allocation = { Device::create_texture(format, size), descriptor, tag };
    uuid id{};

    get()->m_textures_in_use.insert({ id, allocation });
    get()->m_bytes_allocated += byte_size;
    get()->m_bytes_committed += byte_size;

    return id;
  }

  uuid Allocator::allocate_framebuffer(const ivec2 size, const TextureFormat format, const std::string& tag) {
    TextureDescriptor descriptor{ size, format };
    size_t byte_size = descriptor.byte_size();

    for (size_t free_object_index = 0; free_object_index < get()->m_free_objects.size(); free_object_index++) {
      FreeObject& free_object = get()->m_free_objects[free_object_index];
      FreeFramebuffer* free_framebuffer = reinterpret_cast<FreeFramebuffer*>(&free_object.kind);

      if (
        !free_framebuffer ||
        free_framebuffer->allocation.descriptor != descriptor
        ) continue;

      uuid id = free_framebuffer->id;

      free_framebuffer->allocation.tag = tag;
      get()->m_bytes_committed += free_framebuffer->allocation.descriptor.byte_size();
      get()->m_framebuffers_in_use.insert({ id, free_framebuffer->allocation });
      get()->m_free_objects.erase(get()->m_free_objects.begin() + free_object_index);

      return id;
    }

    FramebufferAllocation allocation = { Device::create_framebuffer(Device::create_texture(format, size)), descriptor, tag };
    uuid id{};

    get()->m_framebuffers_in_use.insert({ id, allocation });
    get()->m_bytes_allocated += byte_size;
    get()->m_bytes_committed += byte_size;

    return id;
  }

  void Allocator::purge_if_needed() {
    time_point now = std::chrono::steady_clock::now();

    while (!get()->m_free_objects.empty()) {
      FreeObject& free_object = get()->m_free_objects.front();

      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - free_object.timestamp).count() < DECAY_TIME) break;

      if (free_object.kind.kind == AllocationKind::Buffer) {
        get()->m_bytes_allocated -= reinterpret_cast<FreeGeneralBuffer*>(&free_object.kind)->allocation.size;
      } else if (free_object.kind.kind == AllocationKind::IndexBuffer) {
        get()->m_bytes_allocated -= reinterpret_cast<FreeIndexBuffer*>(&free_object.kind)->allocation.size;
      } else if (free_object.kind.kind == AllocationKind::Texture) {
        get()->m_bytes_allocated -= reinterpret_cast<FreeTexture*>(&free_object.kind)->allocation.descriptor.byte_size();
      } else if (free_object.kind.kind == AllocationKind::Framebuffer) {
        get()->m_bytes_allocated -= reinterpret_cast<FreeFramebuffer*>(&free_object.kind)->allocation.descriptor.byte_size();
      }

      get()->m_free_objects.pop_front();
    }
  }

  void Allocator::free_general_buffer(uuid id) {
    OPTICK_EVENT();

    auto it = get()->m_general_buffers_in_use.find(id);
    if (it == get()->m_general_buffers_in_use.end()) return;

    time_point now = std::chrono::steady_clock::now();

    get()->m_bytes_committed -= it->second.size;
    get()->m_free_objects.push_back(FreeObject{ now, FreeGeneralBuffer{ id, it->second } });
    get()->m_general_buffers_in_use.erase(it);
  }

  void Allocator::free_index_buffer(uuid id) {
    OPTICK_EVENT();

    auto it = get()->m_index_buffers_in_use.find(id);
    if (it == get()->m_index_buffers_in_use.end()) return;

    time_point now = std::chrono::steady_clock::now();

    get()->m_bytes_committed -= it->second.size;
    get()->m_free_objects.push_back(FreeObject{ now, FreeIndexBuffer{ id, it->second } });
    get()->m_index_buffers_in_use.erase(it);
  }

  void Allocator::free_texture(uuid id) {
    OPTICK_EVENT();

    auto it = get()->m_textures_in_use.find(id);
    if (it == get()->m_textures_in_use.end()) return;

    time_point now = std::chrono::steady_clock::now();

    get()->m_bytes_committed -= it->second.descriptor.byte_size();
    get()->m_free_objects.push_back(FreeObject{ now, FreeTexture{ id, it->second } });
    get()->m_textures_in_use.erase(it);
  }

  void Allocator::free_framebuffer(uuid id) {
    OPTICK_EVENT();

    auto it = get()->m_framebuffers_in_use.find(id);
    if (it == get()->m_framebuffers_in_use.end()) return;

    time_point now = std::chrono::steady_clock::now();

    get()->m_bytes_committed -= it->second.descriptor.byte_size();
    get()->m_free_objects.push_back(FreeObject{ now, FreeFramebuffer{ id, it->second } });
    get()->m_framebuffers_in_use.erase(it);
  }

  std::optional<uuid> Allocator::allocate_general_buffer_byte_size(size_t byte_size, const std::string& tag) {
    OPTICK_EVENT();

    if (byte_size < MAX_BUFFER_SIZE_CLASS) {
      byte_size = Math::next_power_of_two(byte_size);
    }

    time_point now = std::chrono::steady_clock::now();

    for (size_t free_object_index = 0; free_object_index < m_free_objects.size(); free_object_index++) {
      FreeObject& free_object = m_free_objects[free_object_index];
      FreeGeneralBuffer* free_general_buffer = reinterpret_cast<FreeGeneralBuffer*>(&free_object.kind);

      if (
        !free_general_buffer ||
        free_general_buffer->allocation.kind != AllocationKind::Buffer ||
        free_general_buffer->allocation.size != byte_size ||
        std::chrono::duration_cast<std::chrono::milliseconds>(now - free_object.timestamp).count() < REUSE_TIME
        ) continue;

      uuid id = free_general_buffer->id;

      free_general_buffer->allocation.tag = tag;
      m_bytes_committed += free_general_buffer->allocation.size;
      m_general_buffers_in_use.insert({ id, free_general_buffer->allocation });
      m_free_objects.erase(m_free_objects.begin() + free_object_index);

      return id;
    }

    return std::nullopt;
  }

  std::optional<uuid> Allocator::allocate_index_buffer_byte_size(size_t byte_size, const std::string& tag) {
    if (byte_size < MAX_BUFFER_SIZE_CLASS) {
      byte_size = Math::next_power_of_two(byte_size);
    }

    time_point now = std::chrono::steady_clock::now();

    for (size_t free_object_index = 0; free_object_index < get()->m_free_objects.size(); free_object_index++) {
      FreeObject& free_object = get()->m_free_objects[free_object_index];
      FreeIndexBuffer* free_index_buffer = reinterpret_cast<FreeIndexBuffer*>(&free_object.kind);

      if (
        !free_index_buffer ||
        free_index_buffer->allocation.kind != AllocationKind::Buffer ||
        free_index_buffer->allocation.size != byte_size ||
        std::chrono::duration_cast<std::chrono::milliseconds>(now - free_object.timestamp).count() < REUSE_TIME
        ) continue;

      uuid id = free_index_buffer->id;

      free_index_buffer->allocation.tag = tag;
      get()->m_bytes_committed += free_index_buffer->allocation.size;
      get()->m_index_buffers_in_use.insert({ id, free_index_buffer->allocation });
      get()->m_free_objects.erase(get()->m_free_objects.begin() + free_object_index);

      return id;
    }

    return std::nullopt;
  }

}
