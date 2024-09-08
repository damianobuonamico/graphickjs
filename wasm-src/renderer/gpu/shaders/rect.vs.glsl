R"(

  precision mediump float;

  uniform highp mat4 u_view_projection;

  in lowp uvec2 a_position;
  in highp vec2 a_instance_position;
  in highp vec2 a_instance_size;
  in lowp uvec4 a_instance_color;

  out vec4 v_color;

  void main() {
    gl_Position = vec4((u_view_projection * vec4(a_instance_position + vec2(a_position) * a_instance_size - a_instance_size / 2.0, 0.0, 1.0)).xyz, 1.0);
    v_color = vec4(a_instance_color) / 255.0;
  }

)"
