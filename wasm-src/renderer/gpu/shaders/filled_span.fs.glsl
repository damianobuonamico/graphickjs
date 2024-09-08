R"(

  precision mediump float;

  in lowp vec4 v_color;

  out vec4 o_frag_color;

  void main() {
    o_frag_color = vec4(v_color.rgb * v_color.a, v_color.a);
  }

)"
