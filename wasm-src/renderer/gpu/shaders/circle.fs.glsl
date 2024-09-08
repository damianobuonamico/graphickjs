R"(

  precision mediump float;

  uniform float u_zoom;

  in lowp vec4 v_color;
  in lowp vec2 v_tex_coord;
  in lowp float v_radius;

  out vec4 o_frag_color;

  void main() {
    float dist = 2.0 * v_radius * length(v_tex_coord);
    float alpha = smoothstep(v_radius, v_radius - 1.0 / u_zoom, dist) ;

    o_frag_color = vec4(v_color.rgb * v_color.a, v_color.a) * alpha;
  }

)"
