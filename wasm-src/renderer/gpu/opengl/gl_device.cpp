/**
 * @file renderer/gpu/opengl/gl_device.cpp
 * @brief The file contains the implementation of the OpenGL device.
 */

#include "gl_device.h"

#include "opengl.h"

#include "../../../utils/resource_manager.h"
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
   * @return The created shader.
   */
  inline static GLuint create_shader(
    const std::string& name, const ShaderKind kind, const std::string& glsl_version_spec,
    const std::vector<std::pair<std::string, std::string>>& variables
  ) {
    const bool is_vertex = kind == ShaderKind::Vertex;

    std::string source = "#version " + glsl_version_spec + "\n" + utils::ResourceManager::get_shader(name + (is_vertex ? ".vs" : ".fs"));

    for (auto& [name, value] : variables) {
      std::string variants[4] = {
        std::string("${") + name + "}",
        std::string("${ ") + name + " }",
        std::string("${") + name + "}",
        std::string("${ ") + name + " }"
      };

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

  void GLDevice::init(const DeviceVersion version) {
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

  void GLDevice::shutdown() {
    if (s_device == nullptr) {
      console::error("Device already shutdown, call init() before shutting down!");
      return;
    }

    delete s_device;
    s_device = nullptr;
  }

  GLDevice::GLDevice(const DeviceVersion version) {
    switch (version) {
    case DeviceVersion::GL3:
      m_glsl_version_spec = "330 core";
      break;
    default:
    case DeviceVersion::GLES3:
      m_glsl_version_spec = "300 es";
      break;
    }

    console::log("Initializing Device:");

    glCall(m_device_name = (const char*)(glGetString(GL_RENDERER)));
    glCall(m_backend_name = (const char*)(glGetString(GL_VERSION)));
    glCall(glGenQueries(1, &m_timer_query));

    console::log("  Device Name", m_device_name);
    console::log("  Backend Name", m_backend_name);

    glCall(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &m_max_vertex_uniform_vectors));

    console::log("Device Initialized!");
  }

  GLDevice::~GLDevice() {
    glCall(glDeleteQueries(1, &m_timer_query));
  }

  void GLDevice::begin_commands() {
#ifndef EMSCRIPTEN
    glCall(glBeginQuery(GL_TIME_ELAPSED, s_device->m_timer_query));
#endif
  }

  size_t GLDevice::end_commands() {
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

  void GLDevice::set_viewport(const irect viewport) {
    if (viewport == s_device->m_state.viewport) {
      return;
    }

    const ivec2 size = viewport.size();

    glCall(glViewport((GLint)viewport.min.x, (GLint)viewport.min.y, (GLsizei)size.x, (GLsizei)size.y));

    s_device->m_state.viewport = viewport;
  }

  void GLDevice::clear(const ClearOps& ops) {
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

  GLProgram GLDevice::create_program(const std::string& name, const std::vector<std::pair<std::string, std::string>>& variables) {
    GLuint vertex = create_shader(name, ShaderKind::Vertex, s_device->m_glsl_version_spec, variables);
    GLuint fragment = create_shader(name, ShaderKind::Fragment, s_device->m_glsl_version_spec, variables);

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

    return GLProgram{ gl_program, vertex, fragment };
  }

  GLUniform GLDevice::get_uniform(const GLProgram& program, const std::string& name) {
    glCall(GLint location = glGetUniformLocation(program.gl_program, name.c_str()));

    if (location < 0) {
      console::error("Uniform " + name + " not found in program!");
      location = 0;
    }

    return GLUniform{ location };
  }

  GLTextureUniform GLDevice::get_texture_uniform(GLProgram& program, const std::string& name) {
    GLUniform uniform = get_uniform(program, name);
    GLuint unit;

    auto it = std::find(program.textures.begin(), program.textures.end(), uniform);

    if (it != program.textures.end()) {
      unit = static_cast<GLuint>(std::distance(program.textures.begin(), it));
    } else {
      unit = static_cast<GLuint>(program.textures.size());
      program.textures.push_back(uniform);
    }

    return GLTextureUniform{ uniform, unit };
  }

  GLVertexAttribute GLDevice::get_vertex_attribute(const GLProgram& program, const std::string& name) {
    glCall(GLint attribute = glGetAttribLocation(program.gl_program, name.c_str()));

    if (attribute < 0) {
      console::error("Attribute " + name + " not found in program!");
      attribute = 0;
    }

    return GLVertexAttribute{ static_cast<GLuint>(attribute) };
  }

  void GLDevice::draw_arrays(const size_t vertex_count, const RenderState& render_state) {
    s_device->set_render_state(render_state);
    glCall(glDrawArrays(gl_primitive(render_state.primitive), 0, (GLsizei)vertex_count));
  }

  void GLDevice::draw_arrays_instanced(const size_t vertex_count, const size_t instance_count, const RenderState& render_state) {
    s_device->set_render_state(render_state);
    glCall(glDrawArraysInstanced(gl_primitive(render_state.primitive), 0, (GLsizei)vertex_count, (GLsizei)instance_count));
  }

  void GLDevice::set_textures(const GLProgram& program, const std::vector<std::pair<GLTextureUniform, const Texture&>>& textures) {
    size_t textures_bound = 0;

    for (const auto& [texture_parameter, texture] : textures) {
      texture.bind(texture_parameter.unit);
      textures_bound |= (size_t)1 << static_cast<size_t>(texture_parameter.unit);
    }

    for (size_t texture_unit = 0; texture_unit < textures.size(); texture_unit++) {
      if ((textures_bound & ((size_t)1 << texture_unit)) != 0) {
        program.textures[texture_unit].set(static_cast<int>(texture_unit));
      }
    }
  }

  void GLDevice::set_uniforms(const GLProgram& program, const std::vector<std::pair<GLUniform, UniformData>>& uniforms) {
    for (const auto& [uniform, data] : uniforms) {
      uniform.set(data);
    }
  }

  void GLDevice::set_render_state(const RenderState& render_state) {
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

    set_textures(render_state.program, render_state.textures);
    set_uniforms(render_state.program, render_state.uniforms);

    if (s_device->m_state.blend != render_state.blend) {
      if (render_state.blend.has_value()) {
        glCall(glEnable(GL_BLEND));
        glCall(glBlendFuncSeparate(
          gl_blend_factor(render_state.blend->src_rgb_factor),
          gl_blend_factor(render_state.blend->dest_rgb_factor),
          gl_blend_factor(render_state.blend->src_alpha_factor),
          gl_blend_factor(render_state.blend->dest_alpha_factor)
        ));
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
      } else {
        glCall(glDisable(GL_DEPTH_TEST));
      }

      s_device->m_state.depth = render_state.depth;
    }

    if (s_device->m_state.stencil != render_state.stencil) {
      if (render_state.stencil.has_value()) {
        glCall(glStencilFunc(
          gl_stencil_func(render_state.stencil->func),
          render_state.stencil->reference,
          render_state.stencil->mask
        ));

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

}
