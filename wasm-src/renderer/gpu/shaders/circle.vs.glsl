R"(

  precision mediump float;

  uniform highp mat4 u_view_projection;

  in lowp uvec2 a_position;
  in highp vec2 a_instance_position;
  in highp float a_instance_radius;
  in lowp uvec4 a_instance_color;

  out lowp vec4 v_color;
  out lowp vec2 v_tex_coord;
  out lowp float v_radius;

  void main() {
    vec2 position = vec2(a_position);

    gl_Position = vec4((u_view_projection * vec4(a_instance_position + 2.0 * position * a_instance_radius - a_instance_radius, 0.0, 1.0)).xyz, 1.0);
    
    v_color = vec4(a_instance_color) / 255.0;
    v_tex_coord = position - 0.5;
    v_radius = a_instance_radius;
  }

)"
