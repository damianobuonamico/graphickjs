R"(

  precision highp float;
  precision highp sampler2D;

  uniform sampler2D uMasksTexture;

  in vec4 vColor;
  in vec2 vMaskCoords;

  out vec4 oFragColor;

  void main() {
    float alpha = texture(uMasksTexture, vMaskCoords).r;
    // oFragColor = mix(vec4(0.9, 0.3, 0.8, 0.5), vColor, alpha);
    oFragColor = vec4(vColor.rgb, vColor.a * alpha * 0.5);
  }

)"
