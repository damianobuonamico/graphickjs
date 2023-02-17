#include "shader_manager.h"

void ShaderManager::create_shaders() {
  static const char instanced_shader_source[] =
    "#vertex\n"
    "uniform mat3 uViewProjectionMatrix;\n"
    "in vec2 aPosition;\n"
    "in vec4 aColor;\n"
    "in vec2 aTranslation;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "  gl_Position = vec4((uViewProjectionMatrix * vec3(aPosition + aTranslation, 1.0)).xy, 0.0, 1.0);\n"
    "  vColor = aColor;\n"
    "}\n"
    "#fragment\n"
    "precision mediump float;\n"
    "in highp vec4 vColor;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "  fragColor = vColor;\n"
    "}\n";

  m_shaders.insert(std::make_pair<std::string, Shader>("instanced", { "instanced", instanced_shader_source }));

  static const char batched_shader_source[] =
    "#vertex\n"
    "uniform mat3 uViewProjectionMatrix;\n"
    "in vec2 aPosition;\n"
    "in vec4 aColor;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "  gl_Position = vec4((uViewProjectionMatrix * vec3(aPosition, 1.0)).xy, 0.0, 1.0);\n"
    "  vColor = aColor;\n"
    "}\n"
    "#fragment\n"
    "precision mediump float;\n"
    "in highp vec4 vColor;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "  fragColor = vColor;\n"
    "}\n";

  m_shaders.insert(std::make_pair<std::string, Shader>("batched", { "batched", batched_shader_source }));

  static const char framebuffer_shader_source[] =
    "#vertex\n"
    "in vec2 aPosition;\n"
    "in vec2 aTexCoords;\n"
    "out vec2 vTexCoords;\n"
    "void main() {\n"
    "  gl_Position = vec4(aPosition, 0.0, 1.0);\n"
    "  vTexCoords = aTexCoords;\n"
    "}\n"
    "#fragment\n"
    "precision mediump float;\n"
    "in vec2 vTexCoords;\n"
    "uniform sampler2D uScreenTexture;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "  fragColor = texture(uScreenTexture, vTexCoords);\n"
    "}\n";

  m_shaders.insert(std::make_pair<std::string, Shader>("framebuffer", { "framebuffer", framebuffer_shader_source }));
}

void ShaderManager::use(const std::string& name) {
  auto iterator = m_shaders.find(name);
  if (iterator != m_shaders.end()) {
    m_current = &iterator->second;
    m_current->use();
    m_current->set_uniform("uViewProjectionMatrix", view_projection_matrix);
  }
}

void ShaderManager::set_uniform(const std::string& name, const int value) {
  if (m_current != nullptr) {
    m_current->set_uniform(name, value);
  }
}

void ShaderManager::set_uniform(const std::string& name, const vec4& value) {
  if (m_current != nullptr) {
    m_current->set_uniform(name, value);
  }
}

void ShaderManager::set_uniform(const std::string& name, const mat3& value) {
  if (m_current != nullptr) {
    m_current->set_uniform(name, value);
  }
}

void ShaderManager::set_global_uniform(const std::string& name, const mat3& value) {
  for (auto i = m_shaders.begin(); i != m_shaders.end(); ++i) {
    i->second.use();
    i->second.set_uniform(name, value);
  }
}

void ShaderManager::set_view_projection_matrix(const mat3& value) {
  view_projection_matrix = value;
}

void ShaderManager::set_attribute(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset) {
  if (m_current != nullptr) {
    m_current->set_attribute(name, size, type, normalized, stride, offset);
  }
}
