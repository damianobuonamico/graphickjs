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

      // TODO: more than these 3 textures should be supported in CPU shader precompilation
      switch (paint_coord) {
      case 1U:
        o_frag_color = texture(u_textures[1], v_tex_coord);
        return;
      case 2U:
        o_frag_color = texture(u_textures[2], v_tex_coord);
        return;
      case 0U:
      default:
        o_frag_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
      }
    } else {
      o_frag_color = v_color + float(v_attr_2) * 0.0000000000000000000000001;
    }
  }

)"
