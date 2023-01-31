#include "shader.h"

Shader::Shader(const std::string& name, const std::string& source)
  : name(name) {
  ShaderSource shader_source = parse_source(source);

  GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, shader_source.vertex.c_str());
  GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, shader_source.fragment.c_str());

  m_program = create_program(vertex_shader, fragment_shader);
}

void Shader::set_uniform(const std::string& name, const mat3& value) {
  GLuint location = get_uniform_location(name);

  glUniformMatrix3fv(location, 1, true, &value);
}

void Shader::set_attribute(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset) {
  GLuint location = get_attribute_location(name);

  glVertexAttribPointer(location, size, type, normalized, stride, offset);
  glEnableVertexAttribArray(location);
}

Shader::ShaderSource Shader::parse_source(const std::string& source) {
  size_t vertex_offset = source.find("#vertex\n");
  size_t fragment_offset = source.find("#fragment\n");


#ifdef EMSCRIPTEN
  ShaderSource shader_source{};
#else
  ShaderSource shader_source{ "#version 330 core\n", "#version 330 core\n" };
#endif

  if (vertex_offset > fragment_offset) {
    shader_source.vertex.append(source.substr(vertex_offset + 8));
    shader_source.fragment.append(source.substr(10, vertex_offset - 10));
  } else {
    shader_source.vertex.append(source.substr(8, fragment_offset - 8));
    shader_source.fragment.append(source.substr(fragment_offset + 10));
  }

  return shader_source;
}

GLuint Shader::create_shader(const GLenum type, const char* source) {
  GLuint shader = glCreateShader(type);

  glShaderSource(shader, 1, &source, nullptr);
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

GLuint Shader::get_uniform_location(const std::string& name) {
  auto iterator = m_locations.find(name);
  if (iterator != m_locations.end()) {
    return iterator->second;
  }

  return m_locations[name] = glGetUniformLocation(m_program, name.c_str());
}

GLuint Shader::get_attribute_location(const std::string& name) {
  auto iterator = m_locations.find(name);
  if (iterator != m_locations.end()) {
    return iterator->second;
  }

  return m_locations[name] = glGetAttribLocation(m_program, name.c_str());
}