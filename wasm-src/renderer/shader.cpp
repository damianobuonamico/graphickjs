#include "shader.h"
#include "../math/mat3.h"

Shader::Shader(const std::string& name, const std::string& source)
  : name(name) {
  ShaderSource shader_source = parse(source);

  GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, shader_source.vertex.c_str());
  GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, shader_source.fragment.c_str());

  m_program = create_program(vertex_shader, fragment_shader);

  mat3 mat1;

  console::log(mat1);

  mat3 mat2(2);

  console::log(mat2);

  mat3 mat = mat1 + mat2;

  console::log(mat);

  mat3 mat4 = mat * mat2;

  console::log(mat4);
}

Shader::ShaderSource Shader::parse(const std::string& source) {
  size_t vertex_offset = source.find("#vertex\n");
  size_t fragment_offset = source.find("#fragment\n");

  ShaderSource shader_source;

  if (vertex_offset > fragment_offset) {
    shader_source.vertex = source.substr(vertex_offset + 8);
    shader_source.fragment = source.substr(10, vertex_offset - 10);
  } else {
    shader_source.vertex = source.substr(8, fragment_offset - 8);
    shader_source.fragment = source.substr(fragment_offset + 10);
  }

  printf("%s", shader_source.vertex.c_str());
  printf("%s", shader_source.fragment.c_str());

  return shader_source;
}

GLuint Shader::create_shader(const GLenum type, const char* source) {
  GLuint shader = glCreateShader(type);

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint is_compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);

  if (!is_compiled) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    char* buf = (char*)malloc(maxLength + 1);
    glGetShaderInfoLog(shader, maxLength, &maxLength, buf);
    printf("%s\n", buf);
    free(buf);

    return 0;
  }

  return shader;
}

GLuint Shader::create_program(const GLuint vertex_shader, const GLuint fragment_shader) {
  GLuint program = glCreateProgram();

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  return program;
}