R"(

  precision highp float;

  uniform float uZoom;

  in vec4 vColor;
  in vec2 vTexCoord;
  in float vLineWidth;

  out vec4 oFragColor;

  void main() {
    float alpha = smoothstep(0.0, 1.25 / uZoom / vLineWidth, 1.0 - abs(vTexCoord.y));

    oFragColor = vColor * alpha;
  }

)"
