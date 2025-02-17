R"(

  precision mediump float;

  uniform float u_zoom;

  in lowp vec4 v_color;
  in lowp vec2 v_tex_coord;
  in lowp float v_parameter;

  flat in highp uint v_attr;

  out vec4 o_frag_color;

  void main() {
    uint type = v_attr & 0xFU;
    float alpha = 1.0;

    switch (type) {
    case 0U:
      float factor = v_parameter * (1.0 - abs(v_tex_coord.y));
      alpha = smoothstep(v_parameter - 1.25, v_parameter, factor);
      
      break;
    case 2U:
      float dist = 2.0 * v_parameter * length(v_tex_coord);
      alpha = smoothstep(v_parameter, v_parameter - 1.0 / u_zoom, dist) ;
      
      break;
    default:
      break;
    }

    o_frag_color = vec4(v_color.rgb * v_color.a, v_color.a) * alpha;
  }

)"
