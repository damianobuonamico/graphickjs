#include "shader_manager.h"

void ShaderManager::create_shaders() {
  static const char pen_shader_source[] = 
    "#vertex\n"
    "uniform mat3 uViewProjectionMatrix;\n"
    "attribute vec2 aPosition;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "  gl_Position = vec4((uViewProjectionMatrix * vec3(aPosition, 1)).xy, 0.0, 1.0);\n"
    "  vColor = vec4(0.1, 0.1, 0.1, 1.0);\n"
    "}\n"
    "#fragment\n"
    "precision mediump float;\n"
    "varying highp vec4 vColor;\n"
    "void main() {\n"
    "  gl_FragColor = vColor;\n"
    "}\n";

  Shader pen_shader("pen", pen_shader_source);
}