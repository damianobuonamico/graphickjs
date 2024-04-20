R"(

  precision highp float;

  uniform float uZoom;

  in vec4 vColor;
  in vec2 vTexCoord;
  in float vLineWidth;

  out vec4 oFragColor;

  void main() {
    float factor = vLineWidth * (1.0 - abs(vTexCoord.y));
    float alpha = smoothstep(vLineWidth - 1.25, vLineWidth, factor);
    // float alpha = smoothstep(0.0, 1.25 / uZoom / vLineWidth, 1.0 - abs(vTexCoord.y));

    // float factor = fwidth(vTexCoord.y);
    // float alpha = smoothstep(0.0, 1.0 / factor, vLineWidth * (1.0 - abs(vTexCoord.y)));
    // 1.0 - smoothstep( 1.0 - ((2.0*u_aa_radius[0]) / v_line_width),  1.0, abs( v_u / v_line_width ) );

    // float aa_factor = 0.5;
    // float alpha = 1.0 - smoothstep(1.0 - ((2.0 * aa_factor) / vLineWidth),  1.0, abs(vTexCoord.y / vLineWidth) );

    oFragColor = vColor * alpha;
  }

)"
