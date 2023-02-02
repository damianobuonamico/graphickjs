#include "shader_manager.h"

void ShaderManager::create_shaders() {
  static const char pen_shader_source[] =
    "#vertex\n"
    "uniform mat3 uViewProjectionMatrix;\n"
    "attribute vec2 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "  gl_Position = vec4((uViewProjectionMatrix * vec3(aPosition, 1.0)).xy, 0.0, 1.0);\n"
    "  vColor = aColor;\n"
    "}\n"
    "#fragment\n"
    "precision mediump float;\n"
    "varying highp vec4 vColor;\n"
    "void main() {\n"
    "  gl_FragColor = vColor;\n"
    "}\n";

  m_shaders.insert(std::make_pair<std::string, Shader>("pen", { "pen", pen_shader_source }));
}

void ShaderManager::use(const std::string& name) {
  auto iterator = m_shaders.find(name);
  if (iterator != m_shaders.end()) {
    m_current = &iterator->second;
    m_current->use();
  }
}

void ShaderManager::set_uniform(const std::string& name, const mat3& value) {
  if (m_current != nullptr) {
    m_current->set_uniform(name, value);
  }
}

void ShaderManager::set_global_uniform(const std::string& name, const mat3& value) {
  for (auto i = m_shaders.begin(); i != m_shaders.end(); ++i) {
    i->second.set_uniform(name, value);
  }
}

void ShaderManager::set_attribute(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset) {
  if (m_current != nullptr) {
    m_current->set_attribute(name, size, type, normalized, stride, offset);
  }
}
