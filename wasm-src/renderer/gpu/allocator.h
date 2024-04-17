/**
 * @file allocator.h
 * @brief The file contains the definition of the GPU memory allocator.
 */

#pragma once

#include "gpu_memory.h"

#include <unordered_map>
#include <deque>

namespace graphick::renderer::GPU::Memory {

  /**
   * @brief The class that represents the GPU memory allocator.
   *
   * The allocator is responsible for allocating and freeing GPU memory. It is also responsible for purging unused memory
   * to avoid unnecessary reallocations.
   *
   * @class Allocator
   */
  class Allocator {
  public:
    Allocator(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    /**
     * @brief Initializes the GPU memory allocator.
     */
    static void init();

    /**
     * @brief Shuts down the GPU memory allocator. It is necessary to call this method before reinitializing the device.
     */
    static void shutdown();

    /**
     * @brief Allocated an empty general buffer of the given size on the GPU.
     *
     * @param size The number of elements in the buffer to allocate.
     * @param tag An identifier to identify the buffer.
     * @return A uuid that can be used to retrieve the buffer.
     */
    template <typename T>
    inline static uuid allocate_general_buffer(const size_t size, const std::string& tag) { return get()->allocate_general_buffer_internal(size * sizeof(T), tag); }
    inline static uuid allocate_byte_general_buffer(const size_t byte_size, const std::string& tag) { return get()->allocate_general_buffer_internal(byte_size, tag); }

    /**
     * @brief Allocated an empty index buffer of the given size on the GPU.
     *
     * @param size The number of elements in the buffer to allocate.
     * @param tag An identifier to identify the buffer.
     * @return A uuid that can be used to retrieve the buffer.
     */
    template <typename T>
    static uuid allocate_index_buffer(const size_t size, const std::string& tag) { return get()->allocate_index_buffer_internal(size * sizeof(T), tag); }
    static uuid allocate_byte_index_buffer(const size_t byte_size, const std::string& tag) { return get()->allocate_index_buffer_internal(byte_size, tag); }

    /**
     * @brief Allocated an empty texture of the given size on the GPU.
     *
     * @param size The dimensions of the texture.
     * @param format The format of the texture to allocate.
     * @param tag An identifier to identify the texture.
     * @return A uuid that can be used to retrieve the texture.
     */
    static uuid allocate_texture(const ivec2 size, const TextureFormat format, const std::string& tag);

    /**
     * @brief Allocated a framebuffer with an empty texture of the given size on the GPU.
     *
     * @param size The dimensions of the framebuffer's texture.
     * @param format The format of the framebuffer's texture.
     * @param tag An identifier to identify the framebuffer.
     * @return A uuid that can be used to retrieve the framebuffer.
     */
    static uuid allocate_framebuffer(const ivec2 size, const TextureFormat format, const std::string& tag);

    /**
     * @brief Purges unused GPU memory.
     */
    static void purge_if_needed();

    /**
     * @brief Frees the buffer with the given uuid.
     *
     * It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     *
     * @param id The uuid of the index buffer to free.
     */
    static void free_general_buffer(uuid id);

    /**
     * @brief Frees the buffer with the given uuid.
     *
     * It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     *
     * @param id The uuid of the general buffer to free.
     */
    static void free_index_buffer(uuid id);

    /**
     * @brief Frees the texture with the given uuid.
     *
     * It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     *
     * @param id The uuid of the texture to free.
     */
    static void free_texture(uuid id);

    /**
     * @brief Frees the framebuffer with the given uuid.
     *
     * It does not really delete the buffer until purge_if_needed() is called
     * to reuse the already allocated memory and avoid unnecessary reallocations.
     *
     * @param id The uuid of the framebuffer to free.
     */
    static void free_framebuffer(uuid id);

    /**
     * @brief Retrieves the general buffer allocated with the given uuid on the GPU.
     *
     * @param id The uuid of the general buffer to retrieve.
     * @return The buffer with the given uuid.
     */
    inline static const Buffer& get_general_buffer(uuid id) { return get()->m_general_buffers_in_use.at(id).get(); }

    /**
     * @brief Retrieves the index buffer allocated with the given uuid on the GPU.
     *
     * @param id The uuid of the index buffer to retrieve.
     * @return The buffer with the given uuid.
     */
    inline static const Buffer& get_index_buffer(uuid id) { return get()->m_index_buffers_in_use.at(id).get(); }

    /**
     * @brief Retrieves the texture allocated with the given uuid on the GPU.
     *
     * @param id The uuid of the texture to retrieve.
     * @return The buffer with the given uuid.
     */
    inline static const Texture& get_texture(uuid id) { return get()->m_textures_in_use.at(id).get(); }

    /**
     * @brief Retrieves the framebuffer allocated with the given uuid on the GPU.
     *
     * @param id The uuid of the framebuffer to retrieve.
     * @return The buffer with the given uuid.
     */
    inline static const Framebuffer& get_framebuffer(uuid id) { return get()->m_framebuffers_in_use.at(id).get(); }

    /**
     * @brief Returns the number of allocated bytes in use on the GPU.
     *
     * @return The number of bytes allocated on the GPU.
     */
    inline static size_t bytes_allocated() { return get()->m_bytes_allocated; }

    /**
     * @brief Returns the number of bytes reserved on the GPU.
     *
     * It includes allocated and reserved only bytes.
     *
     * @return The number of bytes committed on the GPU.
     */
    inline static size_t bytes_committed() { return get()->m_bytes_committed; }
  private:
    /**
     * @brief Default constructor and destructor.
    */
    Allocator() = default;
    ~Allocator() = default;

    /**
     * @brief Retrieves the instance of the GPU memory allocator.
     */
    inline static Allocator* get() { return s_instance; }

    /**
     * @brief Allocates a general buffer of the given size on the GPU.
     *
     * This method is called by the public allocate_general_buffer method.
     *
     * @param byte_size The number of bytes in the buffer to allocate.
     * @param tag An identifier to identify the buffer.
     * @return A uuid that can be used to retrieve the buffer.
     */
    uuid allocate_general_buffer_internal(const size_t byte_size, const std::string& tag);

    /**
     * @brief Allocates an index buffer of the given size on the GPU.
     *
     * This method is called by the public allocate_index_buffer method.
     *
     * @param byte_size The number of bytes in the buffer to allocate.
     * @param tag An identifier to identify the buffer.
     * @return A uuid that can be used to retrieve the buffer.
     */
    uuid allocate_index_buffer_internal(const size_t byte_size, const std::string& tag);

    /**
     * @brief Allocates a general buffer of the given size on the GPU.
     *
     * @param byte_size The number of bytes in the buffer to allocate.
     * @param tag An identifier to identify the buffer.
     * @return A uuid that can be used to retrieve the buffer, std::nullopt if the allocation failed.
     */
    std::optional<uuid> allocate_general_buffer_byte_size(size_t byte_size, const std::string& tag);

    /**
     * @brief Allocates an index buffer of the given size on the GPU.
     *
     * @param byte_size The number of bytes in the buffer to allocate.
     * @param tag An identifier to identify the buffer.
     * @return A uuid that can be used to retrieve the buffer, std::nullopt if the allocation failed.
     */
    std::optional<uuid> allocate_index_buffer_byte_size(size_t byte_size, const std::string& tag);
  private:
    std::unordered_map<uuid, BufferAllocation> m_general_buffers_in_use;      /* The general buffers currently allocated and in use. */
    std::unordered_map<uuid, BufferAllocation> m_index_buffers_in_use;        /* The index buffers currently allocated and in use. */
    std::unordered_map<uuid, TextureAllocation> m_textures_in_use;            /* The textures currently allocated and in use. */
    std::unordered_map<uuid, FramebufferAllocation> m_framebuffers_in_use;    /* The framebuffers currently allocated and in use. */

    std::deque<FreeObject> m_free_objects;                                    /* The free objects that can be reused. */

    size_t m_bytes_committed = 0;                                             /* The number of bytes allocated on the GPU and in use. */
    size_t m_bytes_allocated = 0;                                             /* The number of bytes allocated on the GPU. */
  private:
    static Allocator* s_instance;                                             /* The instance of the GPU memory allocator singleton. */
  };

}
