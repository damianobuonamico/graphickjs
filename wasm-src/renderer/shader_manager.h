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
private:
  std::unordered_map<std::string, Shader> m_shaders;
  // std::string* m_current;
};