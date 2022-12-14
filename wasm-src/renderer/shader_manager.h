#pragma once

#include <unordered_map>

#include "shader.h"

class ShaderManager {
public:
  ShaderManager() = default;
  ShaderManager(const ShaderManager&) = delete;
  ShaderManager(ShaderManager&&) = delete;

  ~ShaderManager() = default;

  void create_shaders();
  void use(const std::string& name);

  void set_uniform(const std::string& name, const mat3& value);
  void set_global_uniform(const std::string& name, const mat3& value);
  void set_attribute(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);
private:
  std::unordered_map<std::string, Shader> m_shaders;
  Shader* m_current = nullptr;
};