R"(

  precision highp float;

  in vec4 vColor;

  out vec4 oFragColor;

  void main() {
    oFragColor = vec4(vColor.rgb, 0.5);
  }

)"
