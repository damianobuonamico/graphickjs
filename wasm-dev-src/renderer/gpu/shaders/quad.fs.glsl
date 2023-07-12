R"(

  precision highp float;
  precision highp sampler2D;

  uniform sampler2D uTexture;

  in vec2 v_texcoord;

  out vec4 oFragColor;

  void main() {
    oFragColor = texture(uTexture, v_texcoord);
    // oFragColor = texture(uTexture, v_texcoord) * 0.0000001 + vec4(1.0, 0.0, 0.0, 1.0);
  }

)"
