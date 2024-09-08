R"(

  precision mediump float;
  precision mediump int;

  uniform highp mat4 u_view_projection;
  uniform lowp vec2 u_viewport_size;
  uniform highp vec4 u_models[${MAX_MODELS}];

  in lowp uvec2 a_position;
  in highp vec2 a_instance_position;
  in highp vec2 a_instance_size;
  in lowp uvec4 a_instance_color;

  in highp uint a_instance_attr_1;
  in highp uint a_instance_attr_2;
  in highp uint a_instance_attr_3;

  out lowp vec4 v_color;
  out lowp vec2 v_tex_coord;

  flat out highp vec2 v_position;
  flat out highp vec2 v_size;
  flat out highp uint v_attr_1;
  flat out highp uint v_attr_2;

  void main() {
    uint model_index = (a_instance_attr_3 & 0xFFFU) * 2U;
    uint z_index = a_instance_attr_3 >> 12U;

    mat3 transform = mat3(
      a_instance_size.x,     0.0,                   0.0,
      0.0,                   a_instance_size.y,     0.0,
      a_instance_position.x, a_instance_position.y, 1.0
    );
    mat4 model = mat4(
      u_models[model_index].x, u_models[model_index + 1U].x, 0.0, 0.0,
      u_models[model_index].y, u_models[model_index + 1U].y, 0.0, 0.0,
      0.0,                     0.0,                          1.0, 0.0,
      u_models[model_index].z, u_models[model_index + 1U].z, 0.0, 1.0
    );

    mat4 m = u_view_projection * model;

    vec2 position = vec2(a_position);
    vec2 p = (transform * vec3(position, 1.0)).xy;
    vec2 n = normalize(position - 0.5);
    
    float w = u_viewport_size.x;
    float h = u_viewport_size.y;
    float s = m[0][3] * p.x + m[1][3] * p.y + m[3][3];
    float t = m[0][3] * n.x + m[1][3] * n.y;
    float u = w * (s * (m[0][0] * n.x + m[1][0] * n.y) - t * (m[0][0] * p.x + m[1][0] * p.y + m[3][0]));
    float v = h * (s * (m[0][1] * n.x + m[1][1] * n.y) - t * (m[0][1] * p.x + m[1][1] * p.y + m[3][1]));

    float d = s * s * (s * t + sqrt(u * u + v * v)) / (u * u + v * v - s * s * t * t);

    vec2 p_prime = p + n * d;

    gl_Position = m * vec4(p_prime, float(z_index) / 1048576.0, 1.0);

    v_color = vec4(a_instance_color) / 255.0;
    v_tex_coord = position + n * d / a_instance_size;

    v_position = a_instance_position;
    v_size = a_instance_size;
    v_attr_1 = a_instance_attr_1;
    v_attr_2 = a_instance_attr_2;
  }

)"
