R"(

  precision highp float;

  uniform mat4 u_view_projection;
  uniform float u_zoom;

  in uvec2 a_position;
  in vec2 a_instance_from;
  in vec2 a_instance_to;
  in float a_instance_width;
  in uvec4 a_instance_color;

  out vec4 v_color;
  out vec2 v_tex_coord;
  out float v_line_width;

  void main() {
    float elongation = 0.25 / u_zoom;

    vec2 pos = vec2(a_position);
    vec2 dir = a_instance_to - a_instance_from;
    vec2 normalized_dir = normalize(dir);
    vec2 normal = a_instance_width * vec2(-normalized_dir.y, normalized_dir.x);
    
    vec2 position = 
      a_instance_from + pos.x * dir + 
      elongation * (2.0 * pos.x - 1.0) * normalized_dir + 
      normal * (1.0 - 2.0 * pos.y) / u_zoom;

    gl_Position = vec4((u_view_projection * vec4(position, 0.0, 1.0)).xyz, 1.0);
    v_color = vec4(a_instance_color) / 255.0;
    v_tex_coord = vec2(1.0, 1.0 - 2.0 * pos.y);
    v_line_width = a_instance_width;
  }

)"
