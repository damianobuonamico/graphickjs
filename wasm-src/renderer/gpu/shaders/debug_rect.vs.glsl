R"(

  precision mediump float;

  uniform highp mat4 u_view_projection;

  in highp vec2 a_position;
  in lowp vec2 a_tex_coord;
  in highp uint a_primitive;
  in lowp uvec4 a_color;

  out lowp vec2 v_tex_coord;
  out lowp vec4 v_color;

  flat out highp uint v_primitive;

  void main() {
    gl_Position = vec4((u_view_projection * vec4(a_position, 0.0, 1.0)).xyz, 1.0);
    
    v_tex_coord = a_tex_coord;
    v_color = vec4(a_color) / 255.0;

    v_primitive = a_primitive;
  }

)"
