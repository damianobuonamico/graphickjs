R"(

  precision highp float;
  precision mediump int;

  uniform highp mat4 u_view_projection;

  in highp vec2 a_position;
  in lowp uvec4 a_color;
  in highp vec2 a_tex_coord;
  in highp vec2 a_tex_coord_curves;
  in highp uint a_attr_1;
  in highp uint a_attr_2;
  in highp uint a_attr_3;

  out lowp vec4 v_color;
  out highp vec2 v_tex_coord;
  out highp vec2 v_tex_coord_curves;

  flat out highp uint v_attr_1;
  flat out highp uint v_attr_2;
  flat out highp uint v_attr_3;

  void main() {
    uint z_index = a_attr_2 >> 12U;

    gl_Position = u_view_projection * vec4(a_position, float(z_index) / 1048576.0, 1.0);
    
    v_color = vec4(a_color) / 255.0;
    v_tex_coord = a_tex_coord;
    v_tex_coord_curves = a_tex_coord_curves;
    // v_tex_coord = vec2(float(a_tex_coord >> 16U), float(a_tex_coord & 0xFFFFU)) / 65536.0;
    // v_tex_coord_curves = vec2(float(a_tex_coord_curves >> 16U), float(a_tex_coord_curves & 0xFFFFU)) / 65536.0;

    v_attr_1 = a_attr_1;
    v_attr_2 = a_attr_2;
    v_attr_3 = a_attr_3;
  }

)"
