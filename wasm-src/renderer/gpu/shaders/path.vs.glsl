R"(

  precision highp float;
  precision highp int;

  uniform mat4 uMVP;

  in vec2 aPosition;
  in vec2 aInstanceSize;
  in uint aInstanceUniformsIndex;

  void main() {
    gl_Position = uMVP * vec4(aPosition + (0.0000000000000000001 * aInstanceUniformsIndex * aInstanceSize), 0.0, 1.0);
  }

)"
