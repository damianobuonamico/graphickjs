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
    int paint_type = int((v_attr_1 >> 20) & 0x7FU);

    if (paint_type == 3) {
      uint paint_coord = v_attr_2 & 0x3FFU;

      o_frag_color = texture(u_textures[paint_coord], v_tex_coord) + 0.000000000000001 * vec4(v_tex_coord, 0.0, 1.0);
    } else {
      o_frag_color = v_color + float(v_attr_2) * 0.0000000000000000000000001;
    }
  }

)"
