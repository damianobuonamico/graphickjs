R"(

  precision highp float;

  uniform float uZoom;

  in vec4 vColor;
  in vec2 vTexCoord;
  in float vRadius;

  out vec4 oFragColor;

  void main() {
    float dist = 2.0 * vRadius * length(vTexCoord);
    float alpha = smoothstep(vRadius, vRadius - 1.0 / uZoom, dist) ;

    oFragColor = vec4(vColor.rgb * vColor.a, vColor.a) * alpha;
  }

)"
