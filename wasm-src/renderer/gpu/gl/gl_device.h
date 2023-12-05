#pragma once

#include "gl_data.h"

namespace Graphick::Renderer::GPU::GL {

  class GLDevice {
  public:
    GLDevice(const DeviceVersion version, const unsigned int default_framebuffer);
    ~GLDevice() = default;

    GLDevice(const GLDevice&) = delete;
    GLDevice(GLDevice&&) = delete;
    GLDevice& operator=(const GLDevice&) = delete;
    GLDevice& operator=(GLDevice&&) = delete;

    inline static std::string backend_name() { return "OpenGL"; }
    static std::string device_name();

    inline void set_default_framebuffer(const GLuint framebuffer) { m_default_framebuffer = framebuffer; }
    void set_viewport(const vec2 size);
    void clear(const ClearOps& ops) const;

    inline std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size) const { return create_texture(format, size, nullptr); }
    inline std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size, const U8TextureData& data) const { return create_texture(format, size, (const void*)data.data); }
    inline std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size, const F32TextureData& data) const { return create_texture(format, size, (const void*)data.data); }

    GLProgram create_program(const std::string& name) const;
    std::unique_ptr<GLVertexArray> create_vertex_array() const;
    std::unique_ptr<GLBuffer> create_buffer(const BufferUploadMode mode) const;
    std::unique_ptr<GLFramebuffer> create_framebuffer(std::unique_ptr<GLTexture> texture) const;

    std::optional<GLVertexAttr> get_vertex_attr(const GLProgram& program, const std::string& name) const;
    std::optional<GLUniform> get_uniform(const GLProgram& program, const std::string& name) const;
    std::optional<GLTextureParameter> get_texture_parameter(GLProgram& program, const std::string& name) const;
    std::optional<GLStorageBuffer> get_storage_buffer(const GLProgram& program, const std::string& name, const uint32_t binding) const;

    void bind_buffer(const GLVertexArray& vertex_array, const GLBuffer& buffer, const BufferTarget target) const;
    template <typename T>
    inline void allocate_buffer(const GLBuffer& buffer, const std::vector<T>& data, const BufferTarget target) const { allocate_buffer_internal(buffer, (const void*)data.data(), data.size * sizeof(T), target); }
    inline void allocate_buffer(const GLBuffer& buffer, const size_t buffer_size, const BufferTarget target) const { allocate_buffer_internal(buffer, nullptr, buffer_size, target); }

    template <typename T>
    void upload_to_buffer(const GLBuffer& buffer, size_t position, const std::vector<T>& data, const BufferTarget target) const { upload_to_buffer_internal(buffer, position, (const void*)data.data(), data.size() * sizeof(T), target); }
    void upload_to_buffer(const GLBuffer& buffer, size_t position, const void* data, size_t size, const BufferTarget target) const { upload_to_buffer_internal(buffer, position, data, size, target); }

    void configure_vertex_attr(const GLVertexArray& vertex_array, const GLVertexAttr attr, const VertexAttrDescriptor& desc) const;

    void upload_to_texture(const GLTexture& texture, const rect& rect, const void* data) const;
    void set_texture_sampling_mode(const GLTexture& texture, const TextureSamplingFlag flags) const;

    void begin_commands() const;
    void end_commands() const;

    void draw_arrays(const size_t vertex_count, const GLRenderState& render_state) const;
    void draw_elements(const size_t index_count, const GLRenderState& render_state) const;
    void draw_elements_instanced(const size_t index_count, const size_t instance_count, const GLRenderState& render_state) const;
  private:
    void set_render_state(const GLRenderState& render_state) const;
    void reset_render_state(const GLRenderState& render_state) const;
    void set_render_options(const RenderOptions& options) const;
    void reset_render_options(const RenderOptions& options) const;

    void set_uniform(const GLUniform& uniform, const UniformData& data) const;

    void set_storage_buffer(const GLStorageBuffer& storage_buffer, const GLBuffer& buffer) const;
    void unset_storage_buffer(const GLStorageBuffer& storage_buffer) const;

    void bind_render_target(const GLRenderTarget target) const;

    inline void bind_vertex_array(const GLVertexArray& vertex_array) const { glCall(glBindVertexArray(vertex_array.gl_vertex_array)); }
    inline void unbind_vertex_array() const { glCall(glBindVertexArray(0)); }

    std::unique_ptr<GLTexture> create_texture(const TextureFormat format, const ivec2 size, const void* data) const;
    void bind_texture(const GLTexture& texture, const uint32_t unit) const;
    void unbind_texture(uint32_t unit) const;
    void bind_textures(
      const GLProgram& program,
      const std::vector<TextureBinding<GLTextureParameter, const GLTexture&>>& texture_bindings
    ) const;

    GLShader create_shader(const std::string& name, const ShaderKind kind) const;
    void use_program(const GLProgram& program) const { glCall(glUseProgram(program.gl_program)); }
    void unuse_program() const { glCall(glUseProgram(0)); }

    void bind_default_framebuffer() const { glCall(glBindFramebuffer(GL_FRAMEBUFFER, m_default_framebuffer)); }
    void bind_framebuffer(const GLFramebuffer& framebuffer) const { glCall(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.gl_framebuffer)); }

    TextureFormat render_target_format(const GLRenderTarget render_target) const;

    void allocate_buffer_internal(const GLBuffer& buffer, const void* data, const size_t size, const BufferTarget target) const;
    void upload_to_buffer_internal(const GLBuffer& buffer, size_t position, const void* data, const size_t size, const BufferTarget target) const;
  private:
    DeviceVersion m_version;
    GLuint m_default_framebuffer;
    std::unique_ptr<GLTexture> m_dummy_texture;

    std::string m_glsl_version_spec;
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
