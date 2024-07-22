/**
 * @file renderer/gpu/opengl/gl_device.cpp
 * @brief The file contains the implementation of the OpenGL device.
 */

#include "gl_device.h"

#include "opengl.h"

#include "../../../utils/resource_manager.h"
#include "../../../utils/console.h"

namespace graphick::renderer::GPU::GL {

  /**
   * @brief The singleton instance of the OpenGL device.
   */
  GLDevice* GLDevice::s_device = nullptr;

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

  void GLDevice::set_viewport(const ivec2 size) {
    if (size == s_device->m_state.viewport_size) {
      return;
    }

    glCall(glViewport(0, 0, (GLsizei)size.x, (GLsizei)size.y));

    s_device->m_state.viewport_size = size;
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

}
