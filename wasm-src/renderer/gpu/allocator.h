#pragma once

#include "gpu_memory.h"

#include <unordered_map>
#include <deque>

namespace Graphick::Render::GPU::Memory {

  class Allocator {
  public:
    Allocator(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    /**
    * Initializes the GPU memory allocator.
    */
    static void init();
    /**
     * Shuts down the GPU memory allocator. It is necessary to call this method before reinitializing the device.
     */
    static void shutdown();

    /**
     * @size: the number of elements in the buffer to allocate.
     * @tag: an identifier to identify the buffer.
     * @return a UUID that can be used to retrieve the buffer.
     * Allocated an empty general buffer of the given size on the GPU.
     */
    template <typename T>
    inline static UUID allocate_general_buffer(const size_t size, const std::string& tag) { return get()->allocate_general_buffer_internal<T>(size, tag); }
    /**
     * @size: the number of elements in the buffer to allocate.
     * @tag: an identifier to identify the buffer.
     * @return a UUID that can be used to retrieve the buffer.
     * Allocated an empty index buffer of the given size on the GPU.
     */
    template <typename T>
    static UUID allocate_index_buffer(const size_t size, const std::string& tag) { return get()->allocate_index_buffer_internal<T>(size, tag); }
    /**
     * @size: the dimensions of the texture.
     * @format: the format of the texture to allocate.
     * @tag: an identifier to identify the texture.
     * @return a UUID that can be used to retrieve the texture.
     * Allocated an empty texture of the given size on the GPU.
     */
    static UUID allocate_texture(const ivec2 size, const TextureFormat format, const std::string& tag);
    /**
     * @size: the dimensions of the framebuffer's texture.
     * @format: the format of the framebuffer's texture.
     * @tag: an identifier to identify the framebuffer.
     * @return a UUID that can be used to retrieve the framebuffer.
     * Allocated a framebuffer with an empty texture of the given size on the GPU.
     */
    static UUID allocate_framebuffer(const ivec2 size, const TextureFormat format, const std::string& tag);

    /**
    * Purges unused GPU memory.
    */
    static void purge_if_needed();

    /**
     * @id: the UUID of the index buffer to free.
     * Frees the buffer with the given UUID. It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     */
    static void free_general_buffer(UUID id);
    /**
     * @id: the UUID of the general buffer to free.
     * Frees the buffer with the given UUID. It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     */
    static void free_index_buffer(UUID id);
    /**
     * @id: the UUID of the texture to free.
     * Frees the texture with the given UUID. It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     */
    static void free_texture(UUID id);
    /**
     * @id: the UUID of the framebuffer to free.
     * Frees the framebuffer with the given UUID. It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     */
    static void free_framebuffer(UUID id);

    /**
     * @id: the UUID of the general buffer to retrieve.
     * @return the buffer with the given UUID.
     * Retrieves the general buffer allocated with the given UUID on the GPU.
     */
    inline static const Buffer& get_general_buffer(UUID id) { return get()->m_general_buffers_in_use.at(id).get(); }
    /**
     * @id: the UUID of the index buffer to retrieve.
     * @return the buffer with the given UUID.
     * Retrieves the index buffer allocated with the given UUID on the GPU.
     */
    inline static const Buffer& get_index_buffer(UUID id) { return get()->m_index_buffers_in_use.at(id).get(); }
    /**
     * @id: the UUID of the texture to retrieve.
     * @return the buffer with the given UUID.
     * Retrieves the texture allocated with the given UUID on the GPU.
     */
    inline static const Texture& get_texture(UUID id) { return get()->m_textures_in_use.at(id).get(); }
    /**
     * @id: the UUID of the framebuffer to retrieve.
     * @return the buffer with the given UUID.
     * Retrieves the framebuffer allocated with the given UUID on the GPU.
     */
    inline static const Framebuffer& get_framebuffer(UUID id) { return get()->m_framebuffers_in_use.at(id).get(); }

    /**
     * @return the number of bytes allocated on the GPU.
     * Returns the number of allocated bytes in use on the GPU.
     */
    inline static size_t bytes_allocated() { return get()->m_bytes_allocated; }
    /**
     * @return the number of bytes committed on the GPU.
     * Returns the number of bytes reserved on the GPU, it includes allocated and reserved only bytes.
     */
    inline static size_t bytes_committed() { return get()->m_bytes_committed; }
  private:
    Allocator() = default;
    ~Allocator() = default;

    inline static Allocator* get() { return s_instance; }

    template <typename T>
    UUID allocate_general_buffer_internal(const size_t size, const std::string& tag) {
      size_t byte_size = size * sizeof(T);
      std::optional<UUID> allocated_id = allocate_general_buffer_byte_size(byte_size, tag);
      if (allocated_id.has_value()) return allocated_id.value();

      BufferAllocation allocation = { Device::create_buffer(BufferUploadMode::Dynamic), byte_size, tag };
      Device::allocate_buffer(*allocation.buffer, byte_size, BufferTarget::Vertex);
      UUID id{};

      m_general_buffers_in_use.insert({ id, allocation });
      m_bytes_allocated += byte_size;
      m_bytes_committed += byte_size;

      return id;
    }

    template <typename T>
    UUID allocate_index_buffer_internal(const size_t size, const std::string& tag) {
      size_t byte_size = size * sizeof(T);
      std::optional<UUID> allocated_id = allocate_index_buffer_byte_size(byte_size, tag);
      if (allocated_id.has_value()) return allocated_id.value();

      BufferAllocation allocation = { Device::create_buffer(BufferUploadMode::Dynamic), byte_size, tag };
      Device::allocate_buffer(*allocation.buffer, byte_size, BufferTarget::Index);
      UUID id{};

      get()->m_index_buffers_in_use.insert({ id, allocation });
      get()->m_bytes_allocated += byte_size;
      get()->m_bytes_committed += byte_size;

      return id;
    }

    std::optional<UUID> allocate_general_buffer_byte_size(size_t byte_size, const std::string& tag);
    std::optional<UUID> allocate_index_buffer_byte_size(size_t byte_size, const std::string& tag);
  private:
    std::unordered_map<UUID, BufferAllocation> m_general_buffers_in_use;
    std::unordered_map<UUID, BufferAllocation> m_index_buffers_in_use;
    std::unordered_map<UUID, TextureAllocation> m_textures_in_use;
    std::unordered_map<UUID, FramebufferAllocation> m_framebuffers_in_use;

    std::deque<FreeObject> m_free_objects;

    size_t m_bytes_committed = 0;
    size_t m_bytes_allocated = 0;
  private:
    static Allocator* s_instance;
  };

}
