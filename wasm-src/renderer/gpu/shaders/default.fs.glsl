R"(

  precision highp float;

  uniform sampler2D uTexture;

  in vec4 vColor;
  in vec2 vTexCoord;

  out vec4 oFragColor;

  void main() {
    // oFragColor = vec4(vTexCoord, vColor.b, vColor.a + texture(uTexture, vTexCoord).r);
    if (vTexCoord.x < 0.0 || vTexCoord.x < 0.0) {
      oFragColor = vColor;
    } else {
      oFragColor = vec4(vColor.rgb, vColor.a * texture(uTexture, vTexCoord).r);
    }
  }

)"
