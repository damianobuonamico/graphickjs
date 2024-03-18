/**
 * @file device.h
 * @brief Contains the main GPU device.
 *
 * Currently the only backend is OpenGL 3.0+, so everything falls back to that.
 */

#pragma once

#if defined(GK_GLES3) || defined(GK_GL3)
#include "gl/gl_device.h"
#else
#include "gl/gl_device.h"
#endif

namespace Graphick::Renderer::GPU {

  /**
   * @brief The device is the main entry point for the GPU rendering. It is responsible for creating and managing the GPU resources.
   *
   * @class Device
   */
  class Device {
  public:
    Device() = delete;
    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;
    ~Device() = delete;

    /**
     * @brief Initializes the device with the given version.
     *
     * Will throw if initialization and compile versions are not compatible.
     *
     * @param version Backend version to initialize the device with.
     */
    static void init(const DeviceVersion version, const unsigned int default_framebuffer);

    /**
     * @brief Shuts down the device.
     *
     * It is necessary to call this method before reinitializing the device.
     */
    static void shutdown();

    /**
     * @brief Returns the current backend name.
     *
     * @return The backend name.
     */
    inline static std::string backend_name() { return s_device->backend_name(); }

    /**
     * @brief Returns the current device name.
     *
     * @return The device name.
     */
    inline static std::string device_name() { return s_device->device_name(); }

    /**
     * @brief Sets the default framebuffer.
     *
     * This method has to be called before any rendering can be done.
     *
     * @param framebuffer The framebuffer id.
     */
    inline static void set_default_framebuffer(const unsigned int framebuffer) { s_device->set_default_framebuffer(framebuffer); }

    /**
     * @brief Sets the viewport size and device pixel ratio.
     *
     * @param size The size of the viewport.
     * @param dpr The device pixel ratio.
     */
    inline static void set_viewport(const vec2 size) { s_device->set_viewport(size); }

    /**
     * @brief Clears the current framebuffer with the given @ops.
     *
     * @param ops Clear operations.
     */
    inline static void clear(const ClearOps& ops) { s_device->clear(ops); }

    /**
     * @brief Creates a texture with the given @format, @size and @data on the GPU.
     *
     * @param format The format of the texture.
     * @param size The integer size of the texture.
     * @param data The pointer to the texture data, pass nullptr to create an empty texture.
     * @return The created texture.
     */
    inline static std::unique_ptr<Texture> create_texture(const TextureFormat format, const ivec2 size) { return s_device->create_texture(format, size); }
    inline static std::unique_ptr<Texture> create_texture(const TextureFormat format, const ivec2 size, const U8TextureData& data) { return s_device->create_texture(format, size, data); }
    inline static std::unique_ptr<Texture> create_texture(const TextureFormat format, const ivec2 size, const F32TextureData& data) { return s_device->create_texture(format, size, data); }

    /**
     * @brief Creates a program with the given name and kind.
     *
     * It calls create_shader() under the hood.
     *
     * @param name The program name to query.
     * @return The created program.
     */
    inline static Program create_program(const std::string& name) { return s_device->create_program(name); }

    /**
     * @brief Creates an empty vertex array.
     *
     * @return The created vertex array.
     */
    inline static std::unique_ptr<VertexArray> create_vertex_array() { return s_device->create_vertex_array(); }

    /**
     * @brief Creates an empty buffer.
     *
     * @param mode The upload mode of the buffer.
     * @return The created buffer.
     */
    inline static std::unique_ptr<Buffer> create_buffer(const BufferUploadMode mode) { return s_device->create_buffer(mode); }

    /**
     * @brief Creates a framebuffer with the given @texture.
     *
     * It takes ownership of the texture and connects it to the created framebuffer.
     *
     * @param texture The framebuffer's texture.
     * @return The created framebuffer.
     */
    inline static std::unique_ptr<Framebuffer> create_framebuffer(std::unique_ptr<Texture> texture) { return s_device->create_framebuffer(std::move(texture)); }

    /**
     * @brief Queries the location of the attribute with the given name in the given program.
     *
     * @param program The program to search the attribute location into.
     * @param name The attribute name to query.
     * @return The location of the given attribute or std::nullopt if not found.
     */
    inline static std::optional<VertexAttr> get_vertex_attr(const Program& program, const std::string& name) { return s_device->get_vertex_attr(program, name); }

    /**
     * @brief Queries the location of the uniform with the given name in the given program.
     *
     * @param program The program to search the uniform location into.
     * @param name The uniform name to query.
     * @return The location of the given uniform or std::nullopt if not found.
     */
    inline static std::optional<Uniform> get_uniform(const Program& program, const std::string& name) { return s_device->get_uniform(program, name); }

    /**
     * @brief Queries the texture parameter with the given name in the given program.
     *
     * @param program The program to search the texture parameter into.
     * @param name The texture parameter name to query.
     * @return The given texture parameter or std::nullopt if not found.
     */
    inline static std::optional<TextureParameter> get_texture_parameter(Program& program, const std::string& name) { return s_device->get_texture_parameter(program, name); }

    /**
     * @brief Queries the location of the storage buffer with the given name in the given program.
     *
     * @param program The program to search the storage buffer location into.
     * @param name The storage buffer name to query.
     * @return The location of the storage buffer or std::nullopt if not found.
     */
    inline static std::optional<StorageBuffer> get_storage_buffer(const Program& program, const std::string& name, const uint32_t binding) { return s_device->get_storage_buffer(program, name, binding); }

    /**
     * @brief Binds the given buffer.
     *
     * @param vertex_array The vertex array controlling the buffer.
     * @param buffer The buffer to bind.
     * @param target The buffer type (vertex, index, storage).
     */
    inline static void bind_buffer(const VertexArray& vertex_array, const Buffer& buffer, const BufferTarget target) { s_device->bind_buffer(vertex_array, buffer, target); }

    /**
     * @brief Allocates the buffer with the given data.
     *
     * It calls glBufferData() under the hood.
     *
     * @param buffer The buffer to allocate to.
     * @param data The data to upload.
     * @param target The buffer type (vertex, index, storage).
     */
    template <typename T>
    inline static void allocate_buffer(const Buffer& buffer, const std::vector<T>& data, const BufferTarget target) { s_device->allocate_buffer(buffer, data, target); }
    inline static void allocate_buffer(const Buffer& buffer, const size_t buffer_size, const BufferTarget target) { s_device->allocate_buffer(buffer, buffer_size, target); }

    /**
     * @brief Uploads the given data with an offset (position) to an already allocated buffer.
     *
     * It calls glBufferSubData() under the hood.
     *
     * @param buffer The buffer to allocate to.
     * @param position The offset of the new data in the buffer.
     * @param data The data to upload at the given location.
     * @param target The buffer type (vertex, index, storage).
     */
    template <typename T>
    inline static void upload_to_buffer(const Buffer& buffer, size_t position, const std::vector<T>& data, const BufferTarget target) { s_device->upload_to_buffer(buffer, position, data, target); }
    inline static void upload_to_buffer(const Buffer& buffer, size_t position, const void* data, size_t size, const BufferTarget target) { s_device->upload_to_buffer(buffer, position, data, size, target); }

    /**
     * @brief Configures, enables and sets divisors for a vertex attribute.
     *
     * @param vertex_array The vertex array.
     * @param attr The vertex attribute to configure.
     * @param desc The configuration parameters.
     */
    inline static void configure_vertex_attr(const VertexArray& vertex_array, const VertexAttr attr, const VertexAttrDescriptor& desc) { s_device->configure_vertex_attr(vertex_array, attr, desc); }

    /**
     * @brief Uploads data to the given @texture at the specified location (rect).
     *
     * @param texture The texture to upload to.
     * @param rect The bounding rect of the new pixels relative to the texture.
     * @param data The data to upload.
     */
    inline static void upload_to_texture(const Texture& texture, const rect& rect, const void* data) { s_device->upload_to_texture(texture, rect, data); }

    /**
     * @brief Sets the sampling flags for the given texture.
     *
     * @param texture The texture to configure.
     * @param flags The sampling flags to set.
     */
    inline static void set_texture_sampling_mode(const Texture& texture, const TextureSamplingFlag flags) { s_device->set_texture_sampling_mode(texture, flags); }

    /**
     * @brief Returns the texture attached to the given framebuffer.
     *
     * @param framebuffer The framebuffer to get the texture from.
     * @return The texture attached to the given framebuffer.
     */
    inline static const Texture& framebuffer_texture(const Framebuffer& framebuffer) { return *framebuffer.texture; }

    /**
     * @brief Returns the format of the given texture.
     *
     * @param texture The texture to get the format from.
     * @return The format of the given texture.
     */
    inline static TextureFormat texture_format(const Texture& texture) { return texture.format; }

    /**
     * @brief Returns the size of the given texture.
     *
     * @param texture The texture to get the size from.
     * @return The size of the given texture.
     */
    inline static ivec2 texture_size(const Texture& texture) { return texture.size; }

    /**
     * @brief Sets up the GPU for receiving commands.
     */
    inline static void begin_commands() { s_device->begin_commands(); }

    /**
     * @brief Flushes the GPU commands.
     */
    inline static void end_commands() { s_device->end_commands(); }

    /**
     * @brief Performs a glDrawArrays() call with the given render_state.
     *
     * @param index_count The number of vertices to draw.
     * @param render_state The render state to use.
     */
    inline static void draw_arrays(const size_t vertex_count, const RenderState& render_state) { s_device->draw_arrays(vertex_count, render_state); }

    /**
     * @brief Performs a glDrawElements() call with the given render_state.
     *
     * @param index_count The number of indices to draw.
     * @param render_state The render state to use.
     */
    inline static void draw_elements(const size_t index_count, const RenderState& render_state) { s_device->draw_elements(index_count, render_state); }
    /**
     * @brief Performs a glDrawElementsInstanced() with the given render_state.
     *
     * @param index_count The number of indices to draw.
     * @param instance_count The number of instances to draw.
     * @param render_state The render state to use.
     */
    inline static void draw_elements_instanced(const size_t index_count, const size_t instance_count, const RenderState& render_state) { s_device->draw_elements_instanced(index_count, instance_count, render_state); }
  private:
    static DeviceBackend* s_device;    /* The device backend. */
  };

}
