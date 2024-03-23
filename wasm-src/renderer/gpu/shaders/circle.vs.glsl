R"(

  precision highp float;

  uniform mat4 uMVP;
  uniform vec4 uColor;
  uniform float uRadius;

  in vec2 aPosition;
  in vec2 aInstancePosition;

  out vec4 vColor;
  out vec2 vTexCoord;

  void main() {
    gl_Position = vec4((uMVP * vec4(aInstancePosition + 2.0 * aPosition * uRadius - uRadius, 0.0, 1.0)).xyz, 1.0);
    
    vColor = uColor;
    vTexCoord = aPosition - 0.5;
  }

)"
