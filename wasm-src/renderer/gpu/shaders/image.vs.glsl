R"(

  precision mediump float;

  uniform highp mat4 u_view_projection;

  in lowp uvec2 a_position;
  in highp vec2 a_instance_position;
  in highp vec2 a_instance_size;

  out lowp vec2 v_tex_coord;

  void main() {
    gl_Position = vec4((u_view_projection * vec4(a_instance_position + vec2(a_position) * a_instance_size - a_instance_size / 2.0, 0.0, 1.0)).xyz, 1.0);
    v_tex_coord = vec2(ivec2(a_position.x, 1U - a_position.y));
  }

)"
