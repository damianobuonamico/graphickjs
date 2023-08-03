R"(

  precision highp float;

  uniform float uRadius;
  uniform float uZoom;

  in vec4 vColor;
  in vec2 vTexCoord;

  out vec4 oFragColor;

  void main() {
    float dist = 2.0 * uRadius * length(vTexCoord);
    float col = smoothstep(uRadius, uRadius - 1.0 / uZoom, dist) ;

    oFragColor = vec4(vColor.rgb, col);
  }

)"
