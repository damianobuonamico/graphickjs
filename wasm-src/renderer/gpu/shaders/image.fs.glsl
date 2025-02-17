R"(

  precision mediump float;
  precision lowp sampler2D;

  uniform sampler2D u_texture;

  in lowp vec2 v_tex_coord;

  out vec4 o_frag_color;

  void main() {
    o_frag_color = texture(u_texture, v_tex_coord);
    // o_frag_color = texture(u_texture, v_tex_coord) + vec4(0.1, 0.0, 0.0, 1.0);
  }

)"
