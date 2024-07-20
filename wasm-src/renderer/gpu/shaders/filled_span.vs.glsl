R"(

  precision highp float;

  uniform mat4 u_view_projection;

  in uvec2 a_position;
  in vec2 a_instance_position;
  in vec2 a_instance_size;
  in uvec4 a_instance_color;

  out vec4 v_color;

  void main() {
    gl_Position = vec4((u_view_projection * vec4(a_instance_position + vec2(a_position) * a_instance_size, 0.0, 1.0)).xyz, 1.0);
    v_color = vec4(a_instance_color) / 255.0;
  }

)"
