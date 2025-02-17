R"(

  precision mediump float;
  precision lowp sampler2D;

  uniform sampler2D u_texture;

  in lowp vec2 v_tex_coord;
  in lowp vec4 v_color;

  flat in highp uint v_primitive;

  out vec4 o_frag_color;

  void main() {
    vec4 color = vec4(v_color.rgb, 1.0) * v_color.a;

    switch (v_primitive) {
    case 1U:
      float alpha = texture(u_texture, v_tex_coord).r;
      o_frag_color = vec4(color.rgb, 1.0) * color.a * alpha;
      break;
    case 0U:
    default:
      o_frag_color = color;
      break;
    }
  }

)"
