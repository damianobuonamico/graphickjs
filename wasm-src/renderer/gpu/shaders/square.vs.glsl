R"(

  precision highp float;

  uniform mat4 uMVP;
  uniform vec4 uColor;
  uniform float uSize;

  in vec2 aPosition;
  in vec2 aInstancePosition;

  out vec4 vColor;

  void main() {
    gl_Position = vec4((uMVP * vec4(aInstancePosition + aPosition * uSize - vec2(uSize, uSize) / 2.0, 0.0, 1.0)).xyz, 1.0);
    vColor = uColor;
  }

)"
