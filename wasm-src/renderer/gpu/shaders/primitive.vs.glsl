R"(

  precision mediump float;

  uniform highp mat4 u_view_projection;
  uniform float u_zoom;

  in lowp uvec2 a_position;
  in highp vec2 a_instance_attr_1;
  in highp vec2 a_instance_attr_2;
  in highp uint a_instance_attr_3;
  in lowp uvec4 a_instance_color;

  out lowp vec4 v_color;
  out lowp vec2 v_tex_coord;
  out lowp float v_parameter;

  flat out highp uint v_attr;

  void main() {
    uint type = a_instance_attr_3 & 0xFU;
    vec2 vertex_pos = vec2(a_position);
    vec2 position;

    if (type == 0U) {
      float width = float(a_instance_attr_3 >> 8) / 1024.0;
      float elongation = 0.25 / u_zoom;

      vec2 dir = a_instance_attr_2 - a_instance_attr_1;
      vec2 normalized_dir = normalize(dir);
      vec2 normal = width * vec2(-normalized_dir.y, normalized_dir.x);

      position = 
        a_instance_attr_1 + vertex_pos.x * dir + 
        elongation * (2.0 * vertex_pos.x - 1.0) * normalized_dir + 
        normal * (1.0 - 2.0 * vertex_pos.y) / u_zoom;
      
      v_tex_coord = vec2(1.0, 1.0 - 2.0 * vertex_pos.y);
      v_parameter = width;
    } else {
      vec2 size = type == 2U ? a_instance_attr_2 * 2.0 : a_instance_attr_2;

      position = a_instance_attr_1 + vertex_pos * size - size / 2.0;

      v_tex_coord = vertex_pos - 0.5;
      v_parameter = a_instance_attr_2.x;
    }

    v_color = vec4(a_instance_color) / 255.0;
    v_attr = a_instance_attr_3;

    gl_Position = vec4((u_view_projection * vec4(position, 0.0, 1.0)).xyz, 1.0);
  }

)"
