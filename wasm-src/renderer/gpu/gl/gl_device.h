/**
 * @file gl_device.h
 * @brief The file contains the definition of the OpenGL GPU device.
 */

#pragma once

#include "gl_data.h"

namespace Graphick::Renderer::GPU::GL {

  /**
   * @brief The class that represents the OpenGL GPU device.
   *
   * The device is responsible for creating and managing GPU resources. It is also responsible for executing GPU commands.
   *
   * @class GLDevice
   */
  class GLDevice {
  public:
    /**
     * @brief Constructs a new GLDevice object.
     *
     * @param version The version of the device.
     * @param default_framebuffer The default framebuffer.
     */
    GLDevice(const DeviceVersion version, const unsigned int default_framebuffer);

    /**
     * @brief Default destructor.
     */
    ~GLDevice() = default;

    /**
     * @brief Deleted copy, move constructor and assignment operators.
     */
    GLDevice(const GLDevice&) = delete;
    GLDevice(GLDevice&&) = delete;
    GLDevice& operator=(const GLDevice&) = delete;
    GLDevice& operator=(GLDevice&&) = delete;

    /**
     * @brief Gets the name of the backend.
     *
     * @return The name of the backend.
     */
    inline static std::string backend_name() { return "OpenGL"; }

    /**
     * @brief Gets the name of the device.
     *
     * @return The name of the device.
     */
    static std::string device_name();

    /**
     * @brief Sets the default framebuffer.
     *
     * @param framebuffer The default framebuffer.
     */
    inline void set_default_framebuffer(const GLuint framebuffer) { m_default_framebuffer = framebuffer; }

    /**
     * @brief Sets the viewport size.
     *
     * @param size The size of the viewport.
     */
    void set_viewport(const vec2 size);

    /**
     * @brief Clears the current render target.
     *
     * @param ops The clear operations.
     */
    void clear(const ClearOps& ops) const;

    /**
     * @brief Creates a new texture.
     *
     * @param format The format of the texture.
     * @param size The size of the texture.
     * @return The new texture.
     */
    inline std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size) const { return create_texture(format, size, nullptr); }

    /**
     * @brief Creates a new U8 texture.
     *
     * @param format The format of the texture.
     * @param size The size of the texture.
     * @param data The data of the texture.
     * @return The new texture.
     */
    inline std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size, const U8TextureData& data) const { return create_texture(format, size, (const void*)data.data); }

    /**
     * @brief Creates a new F32 texture.
     *
     * @param format The format of the texture.
     * @param size The size of the texture.
     * @param data The data of the texture.
     * @return The new texture.
     */
    inline std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size, const F32TextureData& data) const { return create_texture(format, size, (const void*)data.data); }

    /**
     * @brief Creates a new shader program.
     *
     * @param name The name of the program
     * @return The new program.
     */
    GLProgram create_program(const std::string& name) const;

    /**
     * @brief Creates a new vertex array.
     *
     * @return The new vertex array.
     */
    std::unique_ptr<GLVertexArray> create_vertex_array() const;

    /**
     * @brief Creates a new buffer.
     *
     * @param mode The upload mode of the buffer.
     * @return The new buffer.
     */
    std::unique_ptr<GLBuffer> create_buffer(const BufferUploadMode mode) const;

    /**
     * @brief Creates a new framebuffer.
     *
     * @param texture The texture to attach to the framebuffer.
     * @return The new framebuffer.
     */
    std::unique_ptr<GLFramebuffer> create_framebuffer(std::unique_ptr<GLTexture> texture) const;

    /**
     * @brief Creates a new vertex attribute.
     *
     * @param program The program to create the vertex attribute for.
     * @param name The name of the vertex attribute.
     * @return The new vertex attribute.
     */
    std::optional<GLVertexAttr> get_vertex_attr(const GLProgram& program, const std::string& name) const;

    /**
     * @brief Creates a new uniform.
     *
     * @param program The program to create the uniform for.
     * @param name The name of the uniform.
     * @return The new uniform.
     */
    std::optional<GLUniform> get_uniform(const GLProgram& program, const std::string& name) const;

    /**
     * @brief Creates a new texture parameter.
     *
     * @param program The program to create the texture parameter for.
     * @param name The name of the texture parameter.
     * @return The new texture parameter.
     */
    std::optional<GLTextureParameter> get_texture_parameter(GLProgram& program, const std::string& name) const;

    /**
     * @brief Creates a new storage buffer.
     *
     * @param program The program to create the storage buffer for.
     * @param name The name of the storage buffer.
     * @param binding The binding of the storage buffer.
     * @return The new storage buffer.
     */
    std::optional<GLStorageBuffer> get_storage_buffer(const GLProgram& program, const std::string& name, const uint32_t binding) const;

    /**
     * @brief Binds the given vertex array.
     *
     * @param vertex_array The vertex array to bind.
     * @param buffer The buffer to bind.
     * @param target The target of the buffer.
     */
    void bind_buffer(const GLVertexArray& vertex_array, const GLBuffer& buffer, const BufferTarget target) const;

    /**
     * @brief Allocates the given buffer with the given data.
     *
     * @param buffer The buffer to allocate.
     * @param data The data to allocate.
     * @param target The target of the buffer.
     */
    template <typename T>
    inline void allocate_buffer(const GLBuffer& buffer, const std::vector<T>& data, const BufferTarget target) const { allocate_buffer_internal(buffer, (const void*)data.data(), data.size * sizeof(T), target); }

    /**
     * @brief Allocates the given buffer with the given size.
     *
     * @param buffer The buffer to allocate.
     * @param buffer_size The size of the buffer.
     * @param target The target of the buffer.
     */
    inline void allocate_buffer(const GLBuffer& buffer, const size_t buffer_size, const BufferTarget target) const { allocate_buffer_internal(buffer, nullptr, buffer_size, target); }

    /**
     * @brief Uploads the given data to the buffer.
     *
     * @param buffer The buffer to upload to.
     * @param position The position to upload to.
     * @param data The data to upload.
     * @param target The target of the buffer.
     */
    template <typename T>
    void upload_to_buffer(const GLBuffer& buffer, size_t position, const std::vector<T>& data, const BufferTarget target) const { upload_to_buffer_internal(buffer, position, (const void*)data.data(), data.size() * sizeof(T), target); }

    /**
     * @brief Uploads the given data to the buffer.
     *
     * @param buffer The buffer to upload to.
     * @param position The position to upload to.
     * @param data The data to upload.
     * @param size The size of the data.
     * @param target The target of the buffer.
     */
    void upload_to_buffer(const GLBuffer& buffer, size_t position, const void* data, size_t size, const BufferTarget target) const { upload_to_buffer_internal(buffer, position, data, size, target); }

    /**
     * @brief Configures the given vertex attribute.
     *
     * @param vertex_array The vertex array to configure the vertex attribute for.
     * @param attr The vertex attribute to configure.
     * @param desc The descriptor of the vertex attribute.
     */
    void configure_vertex_attr(const GLVertexArray& vertex_array, const GLVertexAttr attr, const VertexAttrDescriptor& desc) const;

    /**
     * @brief Uploads the given data to the texture.
     *
     * @param texture The texture to upload to.
     * @param rect The rectangle to upload to.
     * @param data A pointer to the data to upload.
     */
    void upload_to_texture(const GLTexture& texture, const rect& rect, const void* data) const;

    /**
     * @brief Sets the texture sampling mode.
     *
     * @param texture The texture to set the sampling mode for.
     * @param flags The flags of the sampling mode.
     */
    void set_texture_sampling_mode(const GLTexture& texture, const TextureSamplingFlag flags) const;

    /**
     * @brief Prepares the GPU to execute commands.
     */
    void begin_commands() const;

    /**
     * @brief Flushes the GPU commands.
     */
    void end_commands() const;

    /**
     * @brief Draws the binded vertex array.
     *
     * @param vertex_count The number of vertices to draw.
     * @param render_state The render state to use.
     */
    void draw_arrays(const size_t vertex_count, const GLRenderState& render_state) const;

    /**
     * @brief Draws the binded vertex array with instancing.
     *
     * @param vertex_count The number of vertices to draw.
     * @param instance_count The number of instances to draw.
     * @param render_state The render state to use.
     */
    void draw_arrays_instanced(const size_t vertex_count, const size_t instance_count, const GLRenderState& render_state) const;

    /**
     * @brief Draws the elements of the binded vertex array.
     *
     * @param index_count The number of indices to draw.
     * @param render_state The render state to use.
     */
    void draw_elements(const size_t index_count, const GLRenderState& render_state) const;

    /**
     * @brief Draws the binded vertex array with instancing.
     *
     * @param vertex_count The number of vertices to draw.
     * @param instance_count The number of instances to draw.
     * @param render_state The render state to use.
     */
    void draw_elements_instanced(const size_t index_count, const size_t instance_count, const GLRenderState& render_state) const;
  private:
    /**
     * @brief Sets the render state.
     *
     * @param render_state The render state to set.
     */
    void set_render_state(const GLRenderState& render_state) const;

    /**
     * @brief Resets the render state.
     *
     * @param render_state The render state to reset.
     */
    void reset_render_state(const GLRenderState& render_state) const;

    /**
     * @brief Sets the render options.
     *
     * @param options The render options to set.
     */
    void set_render_options(const RenderOptions& options) const;

    /**
     * @brief Resets the render options.
     *
     * @param options The render options to reset.
     */
    void reset_render_options(const RenderOptions& options) const;

    /**
     * @brief Sets the given uniform with the given data.
     *
     * @param uniform The uniform to set.
     * @param data The data to set the uniform with.
     */
    void set_uniform(const GLUniform& uniform, const UniformData& data) const;

    /**
     * @brief Sets the given storage buffer with the given buffer.
     *
     * @param storage_buffer The storage buffer to set.
     * @param buffer The buffer to set the storage buffer with.
     */
    void set_storage_buffer(const GLStorageBuffer& storage_buffer, const GLBuffer& buffer) const;

    /**
     * @brief Unsets the given storage buffer.
     *
     * @param storage_buffer The storage buffer to unset.
     */
    void unset_storage_buffer(const GLStorageBuffer& storage_buffer) const;

    /**
     * @brief Binds the given render target.
     *
     * @param target The render target to bind.
     */
    void bind_render_target(const GLRenderTarget target) const;

    /**
     * @brief Binds the given vertex array.
     *
     * @param vertex_array The vertex array to bind.
     */
    inline void bind_vertex_array(const GLVertexArray& vertex_array) const { glCall(glBindVertexArray(vertex_array.gl_vertex_array)); }

    /**
     * @brief Unbinds the current vertex array.
     */
    inline void unbind_vertex_array() const { glCall(glBindVertexArray(0)); }

    /**
     * @brief Creates a new texture.
     *
     * @param format The format of the texture.
     * @param size The size of the texture.
     * @param data The data of the texture.
     * @return The new texture.
     */
    std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size, const void* data) const;

    /**
     * @brief Binds the given texture.
     *
     * @param texture The texture to bind.
     * @param unit The texture unit to bind the texture to.
     */
    void bind_texture(const GLTexture& texture, const uint32_t unit) const;

    /**
     * @brief Unbinds the given texture.
     *
     * @param unit The texture unit to unbind the texture from.
     */
    void unbind_texture(uint32_t unit) const;

    /**
     * @brief Binds the given textures.
     *
     * @param program The program to bind the textures to.
     * @param texture_bindings The texture bindings to bind.
     */
    void bind_textures(
      const GLProgram& program,
      const std::vector<TextureBinding<GLTextureParameter, const GLTexture&>>& texture_bindings
    ) const;

    /**
     * @brief Creates a new shader.
     *
     * @param name The name of the shader.
     * @param kind The kind of the shader.
     */
    GLShader create_shader(const std::string& name, const ShaderKind kind) const;

    /**
     * @brief Binds the given shader program.
     *
     * @param program The program to bind.
     */
    void use_program(const GLProgram& program) const { glCall(glUseProgram(program.gl_program)); }

    /**
     * @brief Unbinds the current shader program.
     */
    void unuse_program() const { glCall(glUseProgram(0)); }

    /**
     * @brief Binds the given framebuffer.
     *
     * @param framebuffer The framebuffer to bind.
     */
    void bind_default_framebuffer() const { glCall(glBindFramebuffer(GL_FRAMEBUFFER, m_default_framebuffer)); }

    /**
     * @brief Binds the given framebuffer.
     *
     * @param framebuffer The framebuffer to bind.
     */
    void bind_framebuffer(const GLFramebuffer& framebuffer) const { glCall(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.gl_framebuffer)); }

    /**
     * @brief Retrieves the format of the given render target.
     *
     * @param render_target The render target to retrieve the format of.
     */
    TextureFormat render_target_format(const GLRenderTarget render_target) const;

    /**
     * @brief Allocates the given buffer with the given data.
     *
     * @param buffer The buffer to allocate.
     * @param data The data to allocate.
     * @param size The size of the data.
     * @param target The target of the buffer.
     */
    void allocate_buffer_internal(const GLBuffer& buffer, const void* data, const size_t size, const BufferTarget target) const;

    /**
     * @brief Uploads the given data to the buffer.
     *
     * @param buffer The buffer to upload to.
     * @param position The position to upload to.
     * @param data The data to upload.
     * @param size The size of the data.
     * @param target The target of the buffer.
     */
    void upload_to_buffer_internal(const GLBuffer& buffer, size_t position, const void* data, const size_t size, const BufferTarget target) const;
  private:
    DeviceVersion m_version;                       /* The version of the device. */
    GLuint m_default_framebuffer;                  /* The default framebuffer. */
    std::unique_ptr<GLTexture> m_dummy_texture;    /* The dummy texture. */

    std::string m_glsl_version_spec;               /* The GLSL version specification. */
  };

}

namespace Graphick::Renderer::GPU {

  using DeviceBackend = GL::GLDevice;
  using Texture = GL::GLTexture;
  using Shader = GL::GLShader;
  using Program = GL::GLProgram;
  using VertexArray = GL::GLVertexArray;
  using Buffer = GL::GLBuffer;
  using Framebuffer = GL::GLFramebuffer;
  using VertexAttr = GL::GLVertexAttr;
  using Uniform = GL::GLUniform;
  using TextureParameter = GL::GLTextureParameter;
  using StorageBuffer = GL::GLStorageBuffer;
  using RenderTarget = GL::GLRenderTarget;
  using RenderState = GL::GLRenderState;

}
