#pragma once

#include <unordered_map>
#include <GLES2/gl2.h>

#include "../common.h"

class Shader {
public:
  const UUID id;
  const std::string name;
public:
  Shader(const std::string& name, const std::string& source);
  Shader(const Shader&) = delete;
  Shader(Shader&&) = delete;

  ~Shader() = default;

  void use();

  // void set_uniform(const std::string& name, mat3 value);

private:
  struct ShaderSource {
    std::string vertex;
    std::string fragment;
  };

  static ShaderSource parse(const std::string& source);
  static GLuint create_shader(const GLenum type, const char* source);
  static GLuint create_program(const GLuint vertex_shader, const GLuint fragment_shader);
private:
  GLuint m_program;
  std::unordered_map<std::string, GLuint> m_locations;
};