R"(

  precision highp float;

  uniform float uZoom;

  in vec4 vColor;
  in vec2 vTexCoord;
  in float vLineWidth;

  out vec4 oFragColor;

  void main() {
    float factor = vLineWidth * (1.0 - abs(2.0 * vTexCoord.y - 1.0));
    float alpha = smoothstep(vLineWidth - 1.25 / uZoom, vLineWidth, factor);
    
    oFragColor = vColor * alpha;
  }

)"
