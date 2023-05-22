R"(

  precision highp float;
  precision highp sampler2D;

  uniform sampler2D uMaskTexture;

  in vec4 vColor;
  in vec2 vMaskTexCoord;

  out vec4 oFragColor;

  void main() {
    float alpha = texture(uMaskTexture, vMaskTexCoord).r;
    // oFragColor = vec4(vMaskTexCoord, 0.0, 1.0) * (1.0 - 0.000001* (alpha + vColor.r));
    // oFragColor = mix(vColor, vec4(0.0, 0.0, 0.0, 0.0), alpha);
    // oFragColor = vColor * alpha;
    // oFragColor = vec4(vColor.rgb, alpha * 0.000000000001 + 1.0);
    oFragColor = vec4(vColor.rgb, alpha);
  }

)"
