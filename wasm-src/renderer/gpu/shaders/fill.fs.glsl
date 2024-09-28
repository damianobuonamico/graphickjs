R"(
  precision mediump float;
  precision mediump sampler2D;

  uniform sampler2D u_textures[${MAX_TEXTURES}];

  in lowp vec4 v_color;
  in lowp vec2 v_tex_coord;

  flat in highp uint v_attr_1;
  flat in highp uint v_attr_2;

  out vec4 o_frag_color;

  void main() {
    o_frag_color = v_color + float(v_attr_1 - v_attr_2) * 0.0000000000000000000000001 + v_tex_coord.x * 0.00000000000000000000000000000000000001 * texture(u_textures[0], v_tex_coord);
  }

)"
