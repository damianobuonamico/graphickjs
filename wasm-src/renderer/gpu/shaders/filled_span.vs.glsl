R"(

  precision mediump float;

  uniform highp mat4 u_view_projection;
  uniform highp vec4 u_models[${MAX_MODELS}];

  in lowp uvec2 a_position;
  in highp vec2 a_instance_position;
  in highp vec2 a_instance_size;
  in lowp uvec4 a_instance_color;
  in highp uint a_instance_attr_1;

  out lowp vec4 v_color;

  void main() {
    uint model_index = (a_instance_attr_1 & 0xFFFU) * 2U;
    uint z_index = a_instance_attr_1 >> 12U;

    vec2 v = a_instance_position + vec2(a_position) * a_instance_size;
    vec2 position = vec2(
      u_models[model_index].x * v.x + u_models[model_index].y * v.y + u_models[model_index].z,
      u_models[model_index + 1U].x * v.x + u_models[model_index + 1U].y * v.y + u_models[model_index + 1U].z
    );

    gl_Position = vec4((u_view_projection * vec4(position, float(z_index) / 1048576.0, 1.0)).xyz, 1.0);
    v_color = vec4(a_instance_color) / 255.0;
  }

)"
