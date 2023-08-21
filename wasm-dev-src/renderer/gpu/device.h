#pragma once

#if defined(GK_GLES3) || defined(GK_GL3)
#include "gl/gl_device.h"
#else
// Fallback to GL3/GLES3 for now.
#include "gl/gl_device.h"
#endif

namespace Graphick::Renderer::GPU {

  class Device {
  public:
    Device() = delete;
    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;
    ~Device() = delete;

    /**
     * @version: backend version to initialize the device with.
     * Initializes the device with the given @version. Will throw if initialization and compile versions are not compatible.
     */
    static void init(const DeviceVersion version, const unsigned int default_framebuffer);
    /**
     * Shuts down the device. It is necessary to call this method before reinitializing the device.
     */
    static void shutdown();

    /**
     * @return the current backend name.
     */
    inline static std::string backend_name() { return s_device->backend_name(); }
    /**
     * @return the current backend device.
     */
    inline static std::string device_name() { return s_device->device_name(); }

    /**
     * @framebuffer: the framebuffer id.
     * Sets the default framebuffer, this method has to be called before any rendering can be done.
     */
    inline static void set_default_framebuffer(const unsigned int framebuffer) { s_device->set_default_framebuffer(framebuffer); }
    /**
     * @size: the size of the viewport.
     * @dpr: the device pixel ratio.
     * Sets the viewport size and device pixel ratio.
     */
    inline static void set_viewport(const vec2 size) { s_device->set_viewport(size); }
    /**
     * @ops: clear ops.
     * Clears the current framebuffer with the given @ops.
     */
    inline static void clear(const ClearOps& ops) { s_device->clear(ops); }

    /**
     * @format: the format of the texture.
     * @size: the integer size of the texture.
     * @data: the pointer to the texture data, pass nullptr to create an empty texture.
     * @return the created texture.
     * Creates a texture with the given @format, @size and @data on the GPU.
     */
    inline static std::unique_ptr<Texture> create_texture(const TextureFormat format, const ivec2 size) { return s_device->create_texture(format, size); }
    inline static std::unique_ptr<Texture> create_texture(const TextureFormat format, const ivec2 size, const U8TextureData& data) { return s_device->create_texture(format, size, data); }
    inline static std::unique_ptr<Texture> create_texture(const TextureFormat format, const ivec2 size, const F32TextureData& data) { return s_device->create_texture(format, size, data); }
    /**
     * @name: the program name to query.
     * @return the created program.
     * Creates a program with the given @name and @kind. It calls create_shader() under the hood.
     */
    inline static Program create_program(const std::string& name) { return s_device->create_program(name); }
    /**
     * @return the created vertex array.
     * Creates an empty vertex array.
     */
    inline static std::unique_ptr<VertexArray> create_vertex_array() { return s_device->create_vertex_array(); }
    /**
     * @mode: the upload mode of the buffer.
     * @return the created buffer.
     * Creates an empty buffer.
     */
    inline static std::unique_ptr<Buffer> create_buffer(const BufferUploadMode mode) { return s_device->create_buffer(mode); }
    /**
     * @texture: the framebuffer's texture.
     * @return the created framebuffer.
     * Creates a framebuffer with the given @texture. It takes ownership of the @texture and connects it to the created framebuffer.
     */
    inline static std::unique_ptr<Framebuffer> create_framebuffer(std::unique_ptr<Texture> texture) { return s_device->create_framebuffer(std::move(texture)); }

    /**
     * @program: the program to search the attribute location into.
     * @name: the attribute name to query.
     * @return the location of the given attribute.
     * Queries the location of the attribute with the given @name in the given @program. If the attribute is not found, it returns std::nullopt.
     */
    inline static std::optional<VertexAttr> get_vertex_attr(const Program& program, const std::string& name) { return s_device->get_vertex_attr(program, name); }
    /**
     * @program: the program to search the uniform location into.
     * @name: the uniform name to query.
     * @return the location of the given uniform.
     * Queries the location of the uniform with the given @name in the given @program. If the uniform is not found, it returns std::nullopt.
     */
    inline static std::optional<Uniform> get_uniform(const Program& program, const std::string& name) { return s_device->get_uniform(program, name); }
    /**
     * @program: the program to search the texture parameter into.
     * @name: the texture parameter name to query.
     * @return the given texture parameter.
     * Queries the texture parameter with the given @name in the given @program. If the texture parameter is not found, it returns std::nullopt.
     */
    inline static std::optional<TextureParameter> get_texture_parameter(Program& program, const std::string& name) { return s_device->get_texture_parameter(program, name); }
    /**
     * @program: the program to search the storage buffer location into.
     * @name: the storage buffer name to query.
     * @return the location of the given storage buffer.
     * Queries the location of the storage buffer with the given @name in the given @program. If the storage buffer is not found, it returns std::nullopt.
     */
    inline static std::optional<StorageBuffer> get_storage_buffer(const Program& program, const std::string& name, const uint32_t binding) { return s_device->get_storage_buffer(program, name, binding); }

    /**
     * @vertex_array: the vertex array controlling the @buffer.
     * @buffer: the buffer to bind.
     * @target: the buffer type (vertex, index, storage).
     * Binds the given @buffer.
     */
    inline static void bind_buffer(const VertexArray& vertex_array, const Buffer& buffer, const BufferTarget target) { s_device->bind_buffer(vertex_array, buffer, target); }
    /**
     * @buffer: the buffer to allocate to.
     * @data: the data to upload.
     * @target: the buffer type (vertex, index, storage).
     * Allocates the @buffer with the given @data. It calls glBufferData() under the hood.
     */
    template <typename T>
    inline static void allocate_buffer(const Buffer& buffer, const std::vector<T>& data, const BufferTarget target) { s_device->allocate_buffer(buffer, data, target); }
    inline static void allocate_buffer(const Buffer& buffer, const size_t buffer_size, const BufferTarget target) { s_device->allocate_buffer(buffer, buffer_size, target); }
    /**
     * @buffer: the buffer to allocate to.
     * @position: the offset of the new data in the @buffer.
     * @data: the data to upload at the given @location.
     * @target: the buffer type (vertex, index, storage).
     * Uploads the given @data with an offset (@position) to an already allocated @buffer. It calls glBufferSubData() under the hood.
     */
    template <typename T>
    inline static void upload_to_buffer(const Buffer& buffer, size_t position, const std::vector<T>& data, const BufferTarget target) { s_device->upload_to_buffer(buffer, position, data, target); }
    inline static void upload_to_buffer(const Buffer& buffer, size_t position, const void* data, size_t size, const BufferTarget target) { s_device->upload_to_buffer(buffer, position, data, size, target); }

    /**
     * @vertex_array: the vertex array.
     * @attr: the vertex attribute to configure.
     * @desc: the configuration parameters.
     * Configures, enables and sets divisors for a vertex attribute.
     */
    inline static void configure_vertex_attr(const VertexArray& vertex_array, const VertexAttr attr, const VertexAttrDescriptor& desc) { s_device->configure_vertex_attr(vertex_array, attr, desc); }

    /**
     * @texture: the texture to upload to.
     * @rect: the bounding rect of the new pixels relative to the @texture.
     * @data: the data to upload.
     * Uploads data to the given @texture at the specified location (@rect).
     */
    inline static void upload_to_texture(const Texture& texture, const rect& rect, const void* data) { s_device->upload_to_texture(texture, rect, data); }
    /**
     * @texture: the texture to configure.
     * @flags: the sampling flags to set.
     * Sets the sampling flags for the given @texture.
     */
    inline static void set_texture_sampling_mode(const Texture& texture, const TextureSamplingFlag flags) { s_device->set_texture_sampling_mode(texture, flags); }

    /**
     * @framebuffer: the framebuffer to get the texture from.
     * @return the texture attached to the given @framebuffer.
     * Returns the texture attached to the given @framebuffer.
     */
    inline static const Texture& framebuffer_texture(const Framebuffer& framebuffer) { return *framebuffer.texture; }
    /**
     * @texture: the texture to get the format from.
     * @return the format of the given @texture.
     * Returns the format of the given @texture.
     */
    inline static TextureFormat texture_format(const Texture& texture) { return texture.format; }
    /**
     * @texture: the texture to get the size from.
     * @return the size of the given @texture.
     * Returns the size of the given @texture.
     */
    inline static ivec2 texture_size(const Texture& texture) { return texture.size; }

    /**
     * Sets up the gpu for receiving commands.
     */
    inline static void begin_commands() { s_device->begin_commands(); }
    /**
     * Flushes the gpu commands.
     */
    inline static void end_commands() { s_device->end_commands(); }

    /**
     * @index_count: the number of indices to draw.
     * @render_state: the render state to use.
     * Performs a glDrawArrays() call with the given @render_state.
     */
    inline static void draw_arrays(const size_t index_count, const RenderState& render_state) { s_device->draw_arrays(index_count, render_state); }
    /**
     * @index_count: the number of indices to draw.
     * @render_state: the render state to use.
     * Performs a glDrawElements() call with the given @render_state.
     */
    inline static void draw_elements(const size_t index_count, const RenderState& render_state) { s_device->draw_elements(index_count, render_state); }
    /**
     * @index_count: the number of indices to draw.
     * @instance_count: the number of instances to draw.
     * @render_state: the render state to use.
     * Performs a glDrawElementsInstanced() with the given @render_state.
     */
    inline static void draw_elements_instanced(const size_t index_count, const size_t instance_count, const RenderState& render_state) { s_device->draw_elements_instanced(index_count, instance_count, render_state); }
  private:
    static DeviceBackend* s_device;
  };

}
