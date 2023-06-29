R"(

  precision highp float;

  uniform mat4 uViewProjection;
  uniform vec4 uColor;

  in vec2 aPosition;

  out vec4 vColor;

  void main() {
    gl_Position = vec4((uViewProjection * vec4(aPosition.xy, 0.0, 1.0)).xy, 0.0, 1.0);
    vColor = uColor;
  }

)"
