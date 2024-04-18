R"(

  precision highp float;

  uniform mat4 uViewProjection;

  in vec2 aPosition;
  in vec2 aInstancePosition;
  in vec2 aInstanceSize;
  in uvec4 aInstanceColor;

  out vec4 vColor;

  void main() {
    gl_Position = vec4((uViewProjection * vec4(aInstancePosition + aPosition * aInstanceSize - aInstanceSize / 2.0, 0.0, 1.0)).xyz, 1.0);
    vColor = vec4(aInstanceColor) / 255.0;
  }

)"
