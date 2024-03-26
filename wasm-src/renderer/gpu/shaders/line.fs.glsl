R"(

  precision highp float;

  uniform float uLineWidth;
  uniform float uZoom;

  in vec4 vColor;
  in vec2 vTexCoord;

  out vec4 oFragColor;

  void main() {
    float factor = uLineWidth * (1.0 - abs(2.0 * vTexCoord.y - 1.0));
    float alpha = smoothstep(uLineWidth - 1.25 / uZoom, uLineWidth, factor);
    
    oFragColor = vColor * alpha;
  }

)"
