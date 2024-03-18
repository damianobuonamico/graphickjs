/**
 * @file gl_device.cpp
 * @brief The file contains the implementation of the OpenGL device.
 */

#include "gl_device.h"

#include "../../../utils/resource_manager.h"
#include "../../../utils/console.h"

#define DUMMY_TEXTURE_LENGTH 16

namespace Graphick::Renderer::GPU::GL {

  static constexpr GLint gl_internal_format(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8: return GL_R8;
    case TextureFormat::R32F: return GL_R32F;
      // case TextureFormat::R16F: return GL_R16F;
    case TextureFormat::RGBA8: return GL_RGBA8;
    case TextureFormat::RGBA8UI: return GL_RGBA8UI;
      // case TextureFormat::RGBA16F: return GL_RGBA16F;
    default:
    case TextureFormat::RGBA32F: return GL_RGBA32F;
    }
  }

  static constexpr GLuint gl_format(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8:
    case TextureFormat::R32F: return GL_RED;
      // case TextureFormat::R16F: return GL_RED;
    case TextureFormat::RGBA8: return GL_RGBA;
    case TextureFormat::RGBA8UI: return GL_RGBA_INTEGER;
      // case TextureFormat::RGBA16F:
    default:
    case TextureFormat::RGBA32F: return GL_RGBA;
    }
  }

  static constexpr GLuint gl_type(TextureFormat format) {
    switch (format) {
    case TextureFormat::R8:
    case TextureFormat::RGBA8:
    case TextureFormat::RGBA8UI: return GL_UNSIGNED_BYTE;
      // case TextureFormat::R16F:
      // case TextureFormat::RGBA16F: return GL_HALF_FLOAT;
    default:
    case TextureFormat::R32F: return GL_FLOAT;
    case TextureFormat::RGBA32F: return GL_FLOAT;
    }
  }

  static constexpr GLuint gl_type(VertexAttrType format) {
    switch (format) {
    case VertexAttrType::F32: return GL_FLOAT;
    case VertexAttrType::I8: return GL_BYTE;
    case VertexAttrType::I16: return GL_SHORT;
    case VertexAttrType::I32: return GL_INT;
    case VertexAttrType::U8: return GL_UNSIGNED_BYTE;
    default:
    case VertexAttrType::U16: return GL_UNSIGNED_SHORT;
    }
  }

  static constexpr GLenum gl_access(ImageAccess access) {
    switch (access) {
    case ImageAccess::Read: return GL_READ_ONLY;
    case ImageAccess::Write: return GL_WRITE_ONLY;
    default:
    case ImageAccess::ReadWrite: return GL_READ_WRITE;
    }
  }

  static constexpr GLenum gl_target(BufferTarget target) {
    switch (target) {
    case BufferTarget::Vertex: return GL_ARRAY_BUFFER;
    case BufferTarget::Index: return GL_ELEMENT_ARRAY_BUFFER;
    default:
    case BufferTarget::Storage: return GL_SHADER_STORAGE_BUFFER;
    }
  }

  static constexpr GLenum gl_usage(BufferUploadMode usage) {
    switch (usage) {
    case BufferUploadMode::Static: return GL_STATIC_DRAW;
    case BufferUploadMode::Dynamic: return GL_DYNAMIC_DRAW;
    default:
    case BufferUploadMode::Stream: return GL_STREAM_DRAW;
    }
  }

  static constexpr GLenum gl_primitive(Primitive primitive) {
    switch (primitive) {
    case Primitive::Triangles: return GL_TRIANGLES;
    default:
    case Primitive::Lines: return GL_LINES;
    }
  }

  static constexpr GLenum gl_blend_factor(BlendFactor factor) {
    switch (factor) {
    case BlendFactor::Zero: return GL_ZERO;
    case BlendFactor::One: return GL_ONE;
    case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
    case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DestAlpha: return GL_DST_ALPHA;
    case BlendFactor::OneMinusDestAlpha: return GL_ONE_MINUS_DST_ALPHA;
    default:
    case BlendFactor::DestColor: return GL_DST_COLOR;
    }
  }

  static constexpr GLenum gl_blend_op(BlendOp op) {
    switch (op) {
    case BlendOp::Add: return GL_FUNC_ADD;
    case BlendOp::Subtract: return GL_FUNC_SUBTRACT;
    case BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    case BlendOp::Min: return GL_MIN;
    default:
    case BlendOp::Max: return GL_MAX;
    }
  }

  static constexpr GLenum gl_depth_func(DepthFunc func) {
    switch (func) {
    case DepthFunc::Always: return GL_ALWAYS;
    case DepthFunc::Less: return GL_LESS;
    default:
    case DepthFunc::Lequal: return GL_LEQUAL;
    }
  }

  static constexpr GLenum gl_stencil_func(StencilFunc func) {
    switch (func) {
    case StencilFunc::Always: return GL_ALWAYS;
    default:
    case StencilFunc::Equal: return GL_EQUAL;
    }
  }

  static std::string glsl_version_spec(DeviceVersion version) {
    switch (version) {
    case DeviceVersion::GL3: return "330";
    default:
    case DeviceVersion::GLES3: return "300 es";
    }
  }

  template <typename T>
  static void flip_y(T* pixels, ivec2 size, uint8_t channels) {
    size_t stride = (size_t)size.x * channels;
    T temp = 0;

    for (size_t y = 0; y < (size_t)size.y / 2; y++) {
      size_t y1 = y * stride;
      size_t y2 = ((size_t)size.y - y - 1) * stride;

      for (size_t x = 0; x < stride; x++) {
        temp = pixels[y1 + x];

        pixels[y1 + x] = pixels[y2 + x];
        pixels[y2 + x] = temp;
      }
    }
  }

  GLDevice::GLDevice(const DeviceVersion version, const unsigned int default_framebuffer) :
    m_dummy_texture(std::make_unique<GLTexture>(0, ivec2{ 0 }, TextureFormat::RGBA8)),
    m_version(version), m_default_framebuffer(default_framebuffer), m_glsl_version_spec(glsl_version_spec(version))
  {
    U8TextureData dummy_texture_data{ DUMMY_TEXTURE_LENGTH, DUMMY_TEXTURE_LENGTH, 4 };
    m_dummy_texture = create_texture(TextureFormat::RGBA8, ivec2{ DUMMY_TEXTURE_LENGTH }, dummy_texture_data);
  }

  std::string GLDevice::device_name() {
    return (const char*)glGetString(GL_RENDERER);
  }

  void GLDevice::set_viewport(const vec2 size) {
    glCall(glViewport(0, 0, (GLsizei)std::round(size.x), (GLsizei)std::round(size.y)));
  }

  void GLDevice::clear(const ClearOps& ops) const {
    GLuint flags = 0;

    if (ops.color.has_value()) {
      glCall(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
      glCall(glClearColor(ops.color->r, ops.color->g, ops.color->b, ops.color->a));
      flags |= GL_COLOR_BUFFER_BIT;
    }

    if (ops.depth.has_value()) {
      glCall(glDepthMask(GL_TRUE));
      glCall(glClearDepthf(ops.depth.value()));
      flags |= GL_DEPTH_BUFFER_BIT;
    }

    if (ops.stencil.has_value()) {
      glCall(glStencilMask(std::numeric_limits<GLuint>::max()));
      glCall(glClearStencil((GLint)ops.stencil.value()));
      flags |= GL_STENCIL_BUFFER_BIT;
    }

    if (flags != 0) {
      glCall(glClear(flags));
    }
  }

  GLProgram GLDevice::create_program(const std::string& name) const {
    GLShader vertex_shader = create_shader(name, ShaderKind::Vertex);
    GLShader fragment_shader = create_shader(name, ShaderKind::Fragment);

    GLuint gl_program = glCreateProgram();

    glCall(glAttachShader(gl_program, vertex_shader.gl_shader));
    glCall(glAttachShader(gl_program, fragment_shader.gl_shader));
    glCall(glLinkProgram(gl_program));

    GLint link_status = 0;
    glCall(glGetProgramiv(gl_program, GL_LINK_STATUS, &link_status));

    if (link_status != GL_TRUE) {
      GLint maxLength = 0;
      glCall(glGetProgramiv(gl_program, GL_INFO_LOG_LENGTH, &maxLength));

      char* buf = (char*)malloc(maxLength + 1);
      glCall(glGetProgramInfoLog(gl_program, maxLength, &maxLength, buf));

      console::error("Program " + name + " linking failed", buf);
    }

    return GLProgram{ gl_program, { vertex_shader, fragment_shader }, { {} } };
  }

  std::unique_ptr<GLVertexArray> GLDevice::create_vertex_array() const {
    std::unique_ptr<GLVertexArray> vertex_array = std::make_unique<GLVertexArray>(0);
    glCall(glGenVertexArrays(1, &vertex_array->gl_vertex_array));
    return vertex_array;
  }

  std::unique_ptr<GLBuffer> GLDevice::create_buffer(const BufferUploadMode mode) const {
    OPTICK_EVENT();

    std::unique_ptr<GLBuffer> buffer = std::make_unique<GLBuffer>(0, mode);
    glCall(glGenBuffers(1, &buffer->gl_buffer));

    return buffer;
  }

  std::unique_ptr<GLFramebuffer> GLDevice::create_framebuffer(std::unique_ptr<GLTexture> texture) const {
    std::unique_ptr<GLFramebuffer> framebuffer = std::make_unique<GLFramebuffer>(0, std::move(texture));

    glCall(glGenFramebuffers(1, &framebuffer->gl_framebuffer));
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->gl_framebuffer));

    bind_texture(*framebuffer->texture, 0);

    glCall(glFramebufferTexture2D(
      GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D,
      framebuffer->texture->gl_texture,
      0
    ));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      console::error("Framebuffer is not complete!");
    }

    return framebuffer;
  }

  std::optional<GLVertexAttr> GLDevice::get_vertex_attr(const GLProgram& program, const std::string& name) const {
    glCall(GLint attr = glGetAttribLocation(program.gl_program, name.c_str()));

    if (attr < 0) {
      console::error("Attribute not found in program", name);
      return std::nullopt;
    }

    return VertexAttr{ (GLuint)attr };
  }

  std::optional<GLUniform> GLDevice::get_uniform(const GLProgram& program, const std::string& name) const {
    glCall(GLint location = glGetUniformLocation(program.gl_program, name.c_str()));

    if (location < 0) {
      console::error("Uniform not found in program", name);
      return std::nullopt;
    }

    return GLUniform{ location };
  }

  std::optional<GLTextureParameter> GLDevice::get_texture_parameter(GLProgram& program, const std::string& name) const {
    GLUniform uniform = get_uniform(program, name).value();
    auto& textures = program.parameters.textures;

    auto it = std::find(textures.begin(), textures.end(), uniform);

    GLuint index;

    if (it != textures.end()) {
      index = static_cast<GLuint>(std::distance(textures.begin(), it));
    } else {
      index = static_cast<GLuint>(textures.size());
      textures.push_back(uniform);
    }

    return GLTextureParameter{ uniform, index };
  }

  std::optional<GLStorageBuffer> GLDevice::get_storage_buffer(const GLProgram& program, const std::string& name, const uint32_t binding) const {
    return GLStorageBuffer{ (GLint)binding };
  }

  void GLDevice::bind_buffer(const GLVertexArray& vertex_array, const GLBuffer& buffer, const BufferTarget target) const {
    bind_vertex_array(vertex_array);
    glCall(glBindBuffer(gl_target(target), buffer.gl_buffer));
    unbind_vertex_array();
  }

  void GLDevice::configure_vertex_attr(const GLVertexArray& vertex_array, const GLVertexAttr attr, const VertexAttrDescriptor& desc) const {
    bind_vertex_array(vertex_array);

    GLuint attr_type = gl_type(desc.attr_type);

    if (desc.attr_class == VertexAttrClass::Int) {
      glCall(glVertexAttribIPointer(attr.attr, (GLint)desc.size, attr_type, (GLsizei)desc.stride, (const void*)desc.offset));
    } else {
      bool normalized = desc.attr_class == VertexAttrClass::FloatNorm;
      glCall(glVertexAttribPointer(attr.attr, (GLint)desc.size, attr_type, normalized, (GLsizei)desc.stride, (const void*)desc.offset));
    }

    glCall(glVertexAttribDivisor(attr.attr, desc.divisor));
    glCall(glEnableVertexAttribArray(attr.attr));

    unbind_vertex_array();
  }

  void GLDevice::upload_to_texture(const GLTexture& texture, const rect& rect, const void* data) const {
    OPTICK_EVENT();

    bind_texture(texture, 0);

    GLenum format = gl_format(texture.format);
    GLenum type = gl_type(texture.format);

    ivec2 origin = { (int)rect.min.x, (int)rect.min.y };
    ivec2 size = { (int)rect.size().x, (int)rect.size().y };

    if (origin.x == 0 && origin.y == 0 && size == texture.size) {
      glCall(glTexImage2D(GL_TEXTURE_2D,
        0,
        gl_internal_format(texture.format),
        (GLsizei)texture.size.x,
        (GLsizei)texture.size.y,
        0,
        format,
        type,
        data
      ));
    } else {
      glCall(glTexSubImage2D(GL_TEXTURE_2D,
        0,
        (GLsizei)origin.x,
        (GLsizei)origin.y,
        (GLsizei)size.x,
        (GLsizei)size.y,
        format,
        type,
        data
      ));
    }

    set_texture_sampling_mode(texture, TextureSamplingFlag::None);
  }

  void GLDevice::set_texture_sampling_mode(const GLTexture& texture, const TextureSamplingFlag flags) const {
    OPTICK_EVENT();

    bind_texture(texture, 0);

    // TEMP
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    // glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)flags & (int)TextureSamplingFlag::NearestMin ? GL_NEAREST : GL_LINEAR));
    // glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)flags & (int)TextureSamplingFlag::NearestMag ? GL_NEAREST : GL_LINEAR));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)flags & (int)TextureSamplingFlag::RepeatU ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)flags & (int)TextureSamplingFlag::RepeatV ? GL_REPEAT : GL_CLAMP_TO_EDGE));
  }

  void GLDevice::begin_commands() const {}

  void GLDevice::end_commands() const {
    glCall(glFlush());
  }

  void GLDevice::draw_arrays(const size_t vertex_count, const GLRenderState& render_state) const {
    set_render_state(render_state);
    glCall(glDrawArrays(gl_primitive(render_state.primitive), 0, (GLsizei)vertex_count));
    reset_render_state(render_state);
  }

  void GLDevice::draw_elements(const size_t index_count, const GLRenderState& render_state) const {
    set_render_state(render_state);
    glCall(glDrawElements(gl_primitive(render_state.primitive), (GLsizei)index_count, GL_UNSIGNED_INT, nullptr));
    reset_render_state(render_state);
  }

  void GLDevice::draw_elements_instanced(const size_t index_count, const size_t instance_count, const GLRenderState& render_state) const {
    OPTICK_EVENT();

    set_render_state(render_state);
    glCall(glDrawElementsInstanced(gl_primitive(render_state.primitive), (GLsizei)index_count, GL_UNSIGNED_INT, nullptr, (GLsizei)instance_count));
    reset_render_state(render_state);
  }

  void GLDevice::set_render_state(const GLRenderState& render_state) const {
    OPTICK_EVENT();

    bind_render_target(render_state.target);

    vec2 origin = render_state.viewport.min;
    vec2 size = render_state.viewport.size();
    glCall(glViewport((GLint)origin.x, (GLint)origin.y, (GLsizei)size.x, (GLsizei)size.y));

    if (render_state.options.clear_ops.has_ops()) {
      clear(render_state.options.clear_ops);
    }

    use_program(render_state.program);
    bind_vertex_array(render_state.vertex_array);

    bind_textures(render_state.program, render_state.textures);

    for (auto& [storage_buffer, buffer] : render_state.storage_buffers) {
      set_storage_buffer(storage_buffer, buffer);
    }

    for (auto& [uniform, data] : render_state.uniforms) {
      set_uniform(uniform, data);
    }

    set_render_options(render_state.options);
  }

  void GLDevice::reset_render_state(const GLRenderState& render_state) const {
    OPTICK_EVENT();

    reset_render_options(render_state.options);

    for (auto& [storage_buffer, _] : render_state.storage_buffers) {
      unset_storage_buffer(storage_buffer);
    }

    for (auto& [texture_parameter, _] : render_state.textures) {
      unbind_texture(texture_parameter.texture_unit);
      glCall(glUniform1i(texture_parameter.uniform.location, 0));
    }

    unuse_program();
    unbind_vertex_array();
  }

  void GLDevice::set_render_options(const RenderOptions& options) const {
    if (options.blend.has_value()) {
      glCall(glBlendFuncSeparate(
        gl_blend_factor(options.blend->src_rgb_factor),
        gl_blend_factor(options.blend->dest_rgb_factor),
        gl_blend_factor(options.blend->dest_rgb_factor),
        gl_blend_factor(options.blend->dest_rgb_factor)
      ));
      glCall(glBlendEquation(gl_blend_op(options.blend->op)));
      glCall(glEnable(GL_BLEND));
    } else {
      glCall(glDisable(GL_BLEND));
    }

    if (options.depth.has_value()) {
      glCall(glDepthFunc(gl_depth_func(options.depth->func)));
      glCall(glDepthMask(options.depth->write));
      glCall(glEnable(GL_DEPTH_TEST));
    } else {
      glCall(glDisable(GL_DEPTH_TEST));
    }

    if (options.stencil.has_value()) {
      glCall(glStencilFunc(
        gl_stencil_func(options.stencil->func),
        (GLint)options.stencil->reference,
        options.stencil->mask
      ));

      if (options.stencil->write) {
        glCall(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
        glCall(glStencilMask(options.stencil->write));
      } else {
        glCall(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
        glCall(glStencilMask(0));
      }

      glCall(glEnable(GL_STENCIL_TEST));
    } else {
      glCall(glDisable(GL_STENCIL_TEST));
    }

    bool color_mask = options.color_mask;
    glCall(glColorMask(color_mask, color_mask, color_mask, color_mask));
  }

  void GLDevice::reset_render_options(const RenderOptions& options) const {
    if (options.blend.has_value()) {
      glCall(glDisable(GL_BLEND));
    }

    if (options.depth.has_value()) {
      glCall(glDisable(GL_DEPTH_TEST));
    }

    if (options.stencil.has_value()) {
      glCall(glStencilMask(std::numeric_limits<GLuint>::max()));
      glCall(glDisable(GL_STENCIL_TEST));
    }

    glCall(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
  }

  void GLDevice::set_uniform(const GLUniform& uniform, const UniformData& data) const {
    if (std::holds_alternative<int>(data)) {
      glCall(glUniform1i(uniform.location, std::get<int>(data)));
    } else if (std::holds_alternative<ivec2>(data)) {
      ivec2 vec = std::get<ivec2>(data);
      glCall(glUniform2i(uniform.location, (GLint)vec.x, (GLint)vec.y));
    } else if (std::holds_alternative<float>(data)) {
      glCall(glUniform1f(uniform.location, std::get<float>(data)));
    } else if (std::holds_alternative<vec2>(data)) {
      vec2 vec = std::get<vec2>(data);
      glCall(glUniform2f(uniform.location, vec.x, vec.y));
    } else if (std::holds_alternative<vec4>(data)) {
      vec4 vec = std::get<vec4>(data);
      glCall(glUniform4f(uniform.location, vec.x, vec.y, vec.z, vec.w));
    } else if (std::holds_alternative<mat4>(data)) {
      mat4 mat = std::get<mat4>(data);
      glCall(glUniformMatrix4fv(uniform.location, 1, GL_TRUE, &mat[0].x));
    }
  }

  void GLDevice::set_storage_buffer(const GLStorageBuffer& storage_buffer, const GLBuffer& buffer) const {
    glCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, storage_buffer.location, buffer.gl_buffer));
  }

  void GLDevice::unset_storage_buffer(const GLStorageBuffer& storage_buffer) const {
    glCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, storage_buffer.location, 0));
  }

  void GLDevice::bind_render_target(const GLRenderTarget target) const {
    if (target) {
      bind_framebuffer(*target);
    } else {
      bind_default_framebuffer();
    }
  }

  std::unique_ptr<GLTexture> GLDevice::create_texture(const TextureFormat format, const ivec2 size, const void* data) const {
    std::unique_ptr<GLTexture> texture = std::make_unique<GLTexture>(0, size, format);

    glCall(glGenTextures(1, &texture->gl_texture));
    bind_texture(*texture, 0);
    glCall(glTexImage2D(
      GL_TEXTURE_2D,
      0,
      gl_internal_format(format),
      (GLsizei)size.x,
      (GLsizei)size.y,
      0,
      gl_format(format),
      gl_type(format),
      nullptr
    ));

    set_texture_sampling_mode(*texture, TextureSamplingFlag::None);
    return texture;
  }

  void GLDevice::bind_texture(const GLTexture& texture, const uint32_t unit) const {
    OPTICK_EVENT();

    glCall(glActiveTexture(GL_TEXTURE0 + unit));
    glCall(glBindTexture(GL_TEXTURE_2D, texture.gl_texture));
  }

  void GLDevice::unbind_texture(uint32_t unit) const {
    glCall(glActiveTexture(GL_TEXTURE0 + unit));
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
  }

  void GLDevice::bind_textures(const GLProgram& program, const std::vector<TextureBinding<TextureParameter, const GLTexture&>>& texture_bindings) const {
    size_t textures_bound = 0, images_bound = 0;

    for (auto& [texture_parameter, texture] : texture_bindings) {
      bind_texture(texture, texture_parameter.texture_unit);
      textures_bound |= (size_t)1 << (size_t)texture_parameter.texture_unit;
    }

    for (size_t texture_unit = 0; texture_unit < program.parameters.textures.size(); texture_unit++) {
      if ((textures_bound & ((size_t)1 << texture_unit)) == 0) {
        bind_texture(*m_dummy_texture, (GLuint)texture_unit);
      }

      glCall(glUniform1i(program.parameters.textures[texture_unit].location, (GLint)texture_unit));
    }
  }


  GLShader GLDevice::create_shader(const std::string& name, const ShaderKind kind) const {
    bool is_vertex = kind == ShaderKind::Vertex;

    std::string source = "#version " + m_glsl_version_spec + "\n" + Utils::ResourceManager::get_shader(name + (is_vertex ? ".vs" : ".fs"));
    const char* ptr = source.c_str();

    glCall(GLuint gl_shader = glCreateShader(is_vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER));
    glCall(glShaderSource(gl_shader, 1, &ptr, nullptr));
    glCall(glCompileShader(gl_shader));

    GLint compile_status = 0;
    glCall(glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &compile_status));

    if (compile_status != GL_TRUE) {
      GLint maxLength = 0;
      glCall(glGetShaderiv(gl_shader, GL_INFO_LOG_LENGTH, &maxLength));

      char* buf = (char*)malloc(maxLength + 1);
      glCall(glGetShaderInfoLog(gl_shader, maxLength, &maxLength, buf));

      console::error("Shader " + name + " compilation failed", buf);
    }

    return GLShader{ gl_shader };
  }

  TextureFormat GLDevice::render_target_format(const GLRenderTarget render_target) const {
    if (render_target) {
      return render_target->texture->format;
    } else {
      return TextureFormat::RGBA8;
    }
  }

  void GLDevice::allocate_buffer_internal(const GLBuffer& buffer, const void* data, const size_t size, const BufferTarget target) const {
    OPTICK_EVENT();

    GLenum buffer_target = gl_target(target);
    GLenum buffer_usage = gl_usage(buffer.mode);

    glCall(glBindBuffer(buffer_target, buffer.gl_buffer));
    glCall(glBufferData(buffer_target, size, data, buffer_usage));
  }

  void GLDevice::upload_to_buffer_internal(const GLBuffer& buffer, size_t position, const void* data, const size_t size, const BufferTarget target) const {
    OPTICK_EVENT();

    GLenum buffer_target = gl_target(target);

    glCall(glBindBuffer(buffer_target, buffer.gl_buffer));
    glCall(glBufferSubData(buffer_target, (GLintptr)position, size, data));
  }

}
