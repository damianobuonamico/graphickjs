R"(

  precision highp float;
  precision mediump int;

  uniform highp mat4 u_view_projection;

  in highp vec2 a_position;
  in lowp uvec4 a_color;
  in lowp vec2 a_tex_coord;
  in highp uint a_attr_1;
  in highp uint a_attr_2;

  out lowp vec4 v_color;
  out highp vec2 v_tex_coord;

  flat out highp uint v_attr_1;
  flat out highp uint v_attr_2;

  void main() {
    uint z_index = a_attr_2 >> 12U;

    gl_Position = u_view_projection * vec4(a_position, float(z_index) / 1048576.0, 1.0);
    
    v_color = vec4(a_color) / 255.0;
    v_tex_coord = a_tex_coord;

    v_attr_1 = a_attr_1;
    v_attr_2 = a_attr_2;
  }

)"
