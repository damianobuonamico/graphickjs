R"(

  precision highp float;

  uniform float u_zoom;

  in vec4 v_color;
  in vec2 v_tex_coord;
  in float v_radius;

  out vec4 oFragColor;

  void main() {
    float dist = 2.0 * v_radius * length(v_tex_coord);
    float alpha = smoothstep(v_radius, v_radius - 1.0 / u_zoom, dist) ;

    oFragColor = vec4(v_color.rgb * v_color.a, v_color.a) * alpha;
  }

)"
