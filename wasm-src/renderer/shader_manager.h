#pragma once

#include <unordered_map>

#include "shader.h"

class ShaderManager {
public:
  ShaderManager() = default;
  ShaderManager(const ShaderManager&) = delete;
  ShaderManager(ShaderManager&&) = delete;

  ~ShaderManager() = default;

  inline Shader& current() { return *m_current; }

  void create_shaders();
  void use(const std::string& name);

  void set_uniform(const std::string& name, const mat3& value);
  void set_uniform(const std::string& name, const vec4& value);
  void set_global_uniform(const std::string& name, const mat3& value);
  void set_view_projection_matrix(const mat3& value);
  void set_attribute(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);
private:
  std::unordered_map<std::string, Shader> m_shaders;
  Shader* m_current = nullptr;

  mat3 view_projection_matrix;
};