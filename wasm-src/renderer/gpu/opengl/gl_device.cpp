/**
 * @file renderer/gpu/opengl/gl_device.cpp
 * @brief The file contains the implementation of the OpenGL device.
 */

#include "gl_device.h"

#include "opengl.h"

#include "../../../io/resource_manager.h"

#include "../../../utils/console.h"

namespace graphick::renderer::GPU::GL {

/* -- Static methods -- */

/**
 * @brief Creates a shader with the given name, kind and variables.
 *
 * @param name The name of the shader.
 * @param kind The kind of the shader.
 * @param glsl_version_spec The GLSL version specification.
 * @param variables The variables of the shader.
 * @param max_texture_image_units The maximum number of bound texture image units
 * @return The created shader.
 */
inline static GLuint create_shader(
    const std::string& name,
    const ShaderKind kind,
    const std::string& glsl_version_spec,
    const std::vector<std::pair<std::string, std::string>>& variables,
    const size_t max_texture_image_units)
{
  const bool is_vertex = kind == ShaderKind::Vertex;

  std::string source = "#version " + glsl_version_spec + "\n" +
                       io::ResourceManager::get_shader(name + (is_vertex ? ".vs" : ".fs"));

  if (!is_vertex) {
    const std::string placeholder = "${TEXTURE_CASES}";

    size_t pos = 0;

    while ((pos = source.find(placeholder, pos)) != std::string::npos) {
      std::string cases;

      for (size_t i = 1; i < max_texture_image_units - 2; i++) {
        cases += "case " + std::to_string(i) + "U: return texture(u_textures[" +
                 std::to_string(i) + "], tex_coord);";
      }

      source.replace(pos, placeholder.size(), cases);
      pos += cases.size();
    }
  }

  for (auto& [name, value] : variables) {
    std::string variants[4] = {std::string("${") + name + "}",
                               std::string("${ ") + name + " }",
                               std::string("${") + name + "}",
                               std::string("${ ") + name + " }"};

    for (auto& variant : variants) {
      size_t pos = 0;
      while ((pos = source.find(variant, pos)) != std::string::npos) {
        source.replace(pos, variant.length(), value);
        pos += value.length();
      }
    }
  }

  const char* source_ptr = source.c_str();

  glCall(GLuint gl_shader = glCreateShader(is_vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER));
  glCall(glShaderSource(gl_shader, 1, &source_ptr, nullptr));
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

  return gl_shader;
}

/* -- Static member initialization -- */

GLDevice* GLDevice::s_device = nullptr;

/* -- GLDevice -- */

void GLDevice::init(const DeviceVersion version)
{
  if (s_device != nullptr) {
    console::error("Device already initialized, call shutdown() before reinitializing!");
    return;
  }

  if (version != DeviceVersion::GL3 && version != DeviceVersion::GLES3) {
    console::error("Invalid device version, try using a different version!");
    return;
  }

  s_device = new GLDevice(version);
}

void GLDevice::shutdown()
{
  if (s_device == nullptr) {
    console::error("Device already shutdown, call init() before shutting down!");
    return;
  }

  delete s_device;
  s_device = nullptr;
}

GLDevice::GLDevice(const DeviceVersion version)
{
  switch (version) {
    case DeviceVersion::GL3:
      m_glsl_version_spec = "330 core";
      break;
    default:
    case DeviceVersion::GLES3:
      m_glsl_version_spec = "300 es";
      break;
  }

  console::info("Initializing Device:");

  glCall(m_device_name = (const char*)(glGetString(GL_RENDERER)));
  glCall(m_backend_name = (const char*)(glGetString(GL_VERSION)));
  glCall(glGenQueries(1, &m_timer_query));

  console::info("  Device Name", m_device_name);
  console::info("  Backend Name", m_backend_name);

  glCall(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &m_max_vertex_uniform_vectors));
  glCall(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_max_texture_image_units));

  console::info("Device Initialized!");
}

GLDevice::~GLDevice()
{
  glCall(glDeleteQueries(1, &m_timer_query));
}

void GLDevice::begin_commands()
{
#ifndef EMSCRIPTEN
  glCall(glBeginQuery(GL_TIME_ELAPSED, s_device->m_timer_query));
#endif
}

size_t GLDevice::end_commands()
{
  glCall(glFlush());

#ifndef EMSCRIPTEN
  glCall(glEndQuery(GL_TIME_ELAPSED));

  GLuint64 time;

  glCall(glGetQueryObjectui64v(s_device->m_timer_query, GL_QUERY_RESULT, &time));

  return static_cast<size_t>(time);
#else
  return 0;
#endif
}

void GLDevice::set_viewport(const irect viewport)
{
  if (viewport == s_device->m_state.viewport) {
    return;
  }

  const ivec2 size = viewport.size();

  // TODO: should check which framebuffer is bound and set the viewport accordingly
  if (viewport != s_device->m_state.viewport) {
    glCall(glViewport(
        (GLint)viewport.min.x, (GLint)viewport.min.y, (GLsizei)size.x, (GLsizei)size.y));
    s_device->m_state.viewport = viewport;
  }
}

void GLDevice::set_color_mask(const bool red, const bool green, const bool blue, const bool alpha)
{
  glCall(glColorMask(red, green, blue, alpha));
}

void GLDevice::clear(const ClearOps& ops)
{
  GLuint flags = 0;

  if (ops.color.has_value()) {
    if (ops.color != s_device->m_state.clear_ops.color) {
      glCall(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
      glCall(glClearColor(ops.color->r, ops.color->g, ops.color->b, ops.color->a));

      s_device->m_state.clear_ops.color = ops.color.value();
    }

    flags |= GL_COLOR_BUFFER_BIT;
  }

  if (ops.depth.has_value()) {
    glCall(glDepthMask(GL_TRUE));

    if (ops.depth != s_device->m_state.clear_ops.depth) {
#ifdef EMSCRIPTEN
      glCall(glClearDepthf(ops.depth.value()));
#else
      glCall(glClearDepth(ops.depth.value()));
#endif

      s_device->m_state.clear_ops.depth = ops.depth.value();
    }

    flags |= GL_DEPTH_BUFFER_BIT;
  }

  if (ops.stencil.has_value()) {
    glCall(glStencilMask(std::numeric_limits<GLuint>::max()));
    glCall(glClearStencil((GLint)ops.stencil.value()));

    s_device->m_state.clear_ops.stencil = ops.stencil.value();

    flags |= GL_STENCIL_BUFFER_BIT;
  }

  if (flags != 0) {
    glCall(glClear(flags));
  }
}

GLProgram GLDevice::create_program(
    const std::string& name, const std::vector<std::pair<std::string, std::string>>& variables)
{
  GLuint vertex = create_shader(name,
                                ShaderKind::Vertex,
                                s_device->m_glsl_version_spec,
                                variables,
                                s_device->max_texture_image_units());
  GLuint fragment = create_shader(name,
                                  ShaderKind::Fragment,
                                  s_device->m_glsl_version_spec,
                                  variables,
                                  s_device->max_texture_image_units());

  GLuint gl_program = glCreateProgram();

  glCall(glAttachShader(gl_program, vertex));
  glCall(glAttachShader(gl_program, fragment));
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

  return GLProgram{gl_program, vertex, fragment};
}

GLUniform GLDevice::get_uniform(const GLProgram& program, const std::string& name)
{
  glCall(GLint location = glGetUniformLocation(program.gl_program, name.c_str()));

  if (location < 0) {
    console::error("Uniform " + name + " not found in program!");
    location = 0;
  }

  return GLUniform{location};
}

GLTextureUniform GLDevice::get_texture_uniform(GLProgram& program, const std::string& name)
{
  GLUniform uniform = get_uniform(program, name);
  GLuint unit;

  auto it = std::find(program.textures.begin(), program.textures.end(), uniform);

  if (it != program.textures.end()) {
    unit = static_cast<GLuint>(std::distance(program.textures.begin(), it));
  } else {
    unit = static_cast<GLuint>(program.textures.size());
    program.textures.push_back(uniform);
  }

  return GLTextureUniform{uniform, unit};
}

GLTexturesUniform GLDevice::get_textures_uniform(GLProgram& program,
                                                 const std::string& name,
                                                 const size_t count)
{
  GLUniform uniform = get_uniform(program, name);
  std::vector<GLuint> units(count);

  for (GLuint i = 0; i < units.size(); i++) {
    // auto it = std::find(program.textures.begin(), program.textures.end(), uniform);

    // if (it != program.textures.end()) {
    //   units[i] = static_cast<GLuint>(std::distance(program.textures.begin(), it));
    // } else {
    units[i] = static_cast<GLuint>(program.textures.size());
    program.textures.push_back(uniform);
    // }
  }

  return GLTexturesUniform{uniform, units};
}

GLVertexAttribute GLDevice::get_vertex_attribute(const GLProgram& program, const std::string& name)
{
  glCall(GLint attribute = glGetAttribLocation(program.gl_program, name.c_str()));

  if (attribute < 0) {
    console::error("Attribute " + name + " not found in program!");
    attribute = 0;
  }

  return GLVertexAttribute{static_cast<GLuint>(attribute)};
}

void GLDevice::draw_elements(const size_t index_count, const RenderState& render_state)
{
  s_device->set_render_state(render_state);
  glCall(glDrawElements(
      gl_primitive(render_state.primitive), (GLsizei)index_count, GL_UNSIGNED_SHORT, nullptr));
}

void GLDevice::draw_arrays(const size_t vertex_count, const RenderState& render_state)
{
  s_device->set_render_state(render_state);
  glCall(glDrawArrays(gl_primitive(render_state.primitive), 0, (GLsizei)vertex_count));
}

void GLDevice::draw_arrays_instanced(const size_t vertex_count,
                                     const size_t instance_count,
                                     const RenderState& render_state)
{
  s_device->set_render_state(render_state);
  glCall(glDrawArraysInstanced(
      gl_primitive(render_state.primitive), 0, (GLsizei)vertex_count, (GLsizei)instance_count));
}

void GLDevice::default_framebuffer()
{
  glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void GLDevice::blit_framebuffer(const GLFramebuffer& src,
                                const irect src_rect,
                                const irect dst_rect,
                                const bool reverse)
{
  GLbitfield mask = GL_COLOR_BUFFER_BIT;

  if (src.has_depth) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }

  if (reverse) {
    glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
    glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src.gl_framebuffer));

    glCall(glBlitFramebuffer(dst_rect.min.x,
                             dst_rect.min.y,
                             dst_rect.max.x,
                             dst_rect.max.y,
                             src_rect.min.x,
                             src_rect.min.y,
                             src_rect.max.x,
                             src_rect.max.y,
                             mask,
                             GL_NEAREST));
  } else {
    glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, src.gl_framebuffer));
    glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));

    glCall(glBlitFramebuffer(src_rect.min.x,
                             src_rect.min.y,
                             src_rect.max.x,
                             src_rect.max.y,
                             dst_rect.min.x,
                             dst_rect.min.y,
                             dst_rect.max.x,
                             dst_rect.max.y,
                             mask,
                             GL_NEAREST));
  }

  glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void GLDevice::blit_framebuffer(const GLFramebuffer& src,
                                const GLFramebuffer& dst,
                                const irect src_rect,
                                const irect dst_rect)
{
  glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, src.gl_framebuffer));
  glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.gl_framebuffer));

  GLbitfield mask = GL_COLOR_BUFFER_BIT;

  if (src.has_depth && dst.has_depth) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }

  glCall(glBlitFramebuffer(src_rect.min.x,
                           src_rect.min.y,
                           src_rect.max.x,
                           src_rect.max.y,
                           dst_rect.min.x,
                           dst_rect.min.y,
                           dst_rect.max.x,
                           dst_rect.max.y,
                           mask,
                           GL_NEAREST));
}

void GLDevice::set_uniforms(const GLProgram& program, const std::vector<UniformBinding>& uniforms)
{
  for (const auto& [uniform, data] : uniforms) {
    uniform.set(data);
  }
}

void GLDevice::set_textures(const GLProgram& program, const std::vector<TextureBinding>& textures)
{
  for (const auto& [texture_uniform, texture] : textures) {
    texture->bind(texture_uniform.unit);
    program.textures[texture_uniform.unit].set(static_cast<int>(texture_uniform.unit));
  }
}

void GLDevice::set_texture_arrays(const GLProgram& program,
                                  const std::vector<TextureArrayBinding>& texture_arrays)
{
  for (const auto& [textures_uniform, textures] : texture_arrays) {
    std::vector<int> units(textures.size());

    for (int i = 0; i < textures.size(); i++) {
      textures[i]->bind(textures_uniform.units[i]);
      units[i] = textures_uniform.units[i];
    }

    units.resize(textures_uniform.units.size(), units.back());

    program.textures[textures_uniform.units[0]].set(units);
  }
}

void GLDevice::set_render_state(const RenderState& render_state)
{
  const GLuint program = render_state.program.gl_program;

  set_viewport(render_state.viewport);
  clear(render_state.clear_ops);

  if (s_device->m_state.program.gl_program != render_state.program.gl_program) {
    render_state.program.use();
    s_device->m_state.program = render_state.program;
  }

  if (s_device->m_state.vertex_array != render_state.vertex_array) {
    if (render_state.vertex_array != nullptr) {
      render_state.vertex_array->bind();
    } else {
      glCall(glBindVertexArray(0));
    }

    s_device->m_state.vertex_array = render_state.vertex_array;
  }

  set_uniforms(render_state.program, render_state.uniforms);
  set_textures(render_state.program, render_state.textures);
  set_texture_arrays(render_state.program, render_state.texture_arrays);

  if (s_device->m_state.blend != render_state.blend) {
    if (render_state.blend.has_value()) {
      glCall(glEnable(GL_BLEND));
      glCall(glBlendFuncSeparate(gl_blend_factor(render_state.blend->src_rgb_factor),
                                 gl_blend_factor(render_state.blend->dest_rgb_factor),
                                 gl_blend_factor(render_state.blend->src_alpha_factor),
                                 gl_blend_factor(render_state.blend->dest_alpha_factor)));
      glCall(glBlendEquation(gl_blend_op(render_state.blend->op)));
      glCall(glEnable(GL_BLEND));
    } else {
      glCall(glDisable(GL_BLEND));
    }

    s_device->m_state.blend = render_state.blend;
  }

  if (s_device->m_state.depth != render_state.depth) {
    if (render_state.depth.has_value()) {
      glCall(glEnable(GL_DEPTH_TEST));
      glCall(glDepthFunc(gl_depth_func(render_state.depth->func)));
      glCall(glDepthMask(render_state.depth->write));
    } else {
      glCall(glDisable(GL_DEPTH_TEST));
    }

    s_device->m_state.depth = render_state.depth;
  }

  if (s_device->m_state.stencil != render_state.stencil) {
    if (render_state.stencil.has_value()) {
      glCall(glStencilFunc(gl_stencil_func(render_state.stencil->func),
                           render_state.stencil->reference,
                           render_state.stencil->mask));

      if (render_state.stencil->write) {
        glCall(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
        glCall(glStencilMask(render_state.stencil->write));
      } else {
        glCall(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
        glCall(glStencilMask(0));
      }

      glCall(glEnable(GL_STENCIL_TEST));
    } else {
      glCall(glDisable(GL_STENCIL_TEST));
    }

    s_device->m_state.stencil = render_state.stencil;
  }
}
}  // namespace graphick::renderer::GPU::GL
