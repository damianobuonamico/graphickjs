R"(

  precision mediump float;

  uniform float u_zoom;

  in lowp vec4 v_color;
  in lowp vec2 v_tex_coord;
  in lowp float v_line_width;

  out vec4 o_frag_color;

  void main() {
    float factor = v_line_width * (1.0 - abs(v_tex_coord.y));
    float alpha = smoothstep(v_line_width - 1.25, v_line_width, factor);
    // float alpha = smoothstep(0.0, 1.25 / u_zoom / v_line_width, 1.0 - abs(v_tex_coord.y));

    // float factor = fwidth(v_tex_coord.y);
    // float alpha = smoothstep(0.0, 1.0 / factor, v_line_width * (1.0 - abs(v_tex_coord.y)));
    // 1.0 - smoothstep( 1.0 - ((2.0*u_aa_radius[0]) / v_line_width),  1.0, abs( v_u / v_line_width ) );

    // float aa_factor = 0.5;
    // float alpha = 1.0 - smoothstep(1.0 - ((2.0 * aa_factor) / v_line_width),  1.0, abs(v_tex_coord.y / v_line_width) );

    o_frag_color = vec4(v_color.rgb * v_color.a, v_color.a) * alpha;
  }

)"
