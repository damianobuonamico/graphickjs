#pragma once

#include "../common.h"
#include "../math/mat3.h"
#include "../math/vec4.h"

#include <unordered_map>
#ifdef EMSCRIPTEN
#include <GLES2/gl2.h>
#else
#include <glad/glad.h>
#endif

class Shader {
public:
  const UUID id;
  const std::string name;
public:
  Shader(const std::string& name, const std::string& source);
  Shader(const Shader&) = default;
  Shader(Shader&&) = delete;

  ~Shader() = default;

  inline void use() { glUseProgram(m_program); }

  void set_uniform(const std::string& name, const int value);
  void set_uniform(const std::string& name, const vec4& value);
  void set_uniform(const std::string& name, const mat3& value);
  void set_attribute(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);
private:
  struct ShaderSource {
    std::string vertex;
    std::string fragment;
  };

  static ShaderSource parse_source(const std::string& source);
  static GLuint create_shader(const GLenum type, const char* source);
  static GLuint create_program(const GLuint vertex_shader, const GLuint fragment_shader);

  GLuint get_uniform_location(const std::string& name);
  GLuint get_attribute_location(const std::string& name);
private:
  GLuint m_program;
  std::unordered_map<std::string, GLuint> m_locations;
};