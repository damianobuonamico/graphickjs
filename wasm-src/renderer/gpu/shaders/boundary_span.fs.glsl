R"(
  precision highp float;
  precision highp int;
  precision highp sampler2D;

  uniform sampler2D u_curves_texture;
  uniform int u_max_samples;

  in vec4 v_color;
  in vec2 v_tex_coord;

  flat in vec2 v_position;
  flat in vec2 v_size;
  flat in uint v_attr_1;
  flat in uint v_attr_2;

  out vec4 o_frag_color;

  #define to_coords(x) (vec2((float((x) % 512U) + 0.5) / 512.0, (float((x) / 512U) + 0.5) / 512.0))

  float calculate_cubic_root(float a, float b, float c, float d, float t0) {
    float t = t0;

    float a1 = 3.0 * a;
    float b1 = 2.0 * b;
    float a2 = 2.0 * a1;

    for (int i = 0; i < 3; i++) {
      float t_sq = t * t;
      float f = a * t_sq * t + b * t_sq + c * t + d;
      float f_prime = a1 * t_sq + b1 * t + c;
      float f_second = a2 * t + b1;
      
      t = t - 3.0 * f * (3.0 * f_prime * f_prime - f * f_second) / 
        (9.0 * f_prime * f_prime * f_prime - 9.0 * f * f_prime * f_second + f * f * a2);
    }

    return t;
  }

  float cubic_horizontal_coverage(vec2 pixel_pos, float inv_pixel_size, uint curves_offset, uint curves_count) {
    float coverage = 0.0;

    vec2 position_delta = v_position / v_size + pixel_pos;

    for (uint curve = curves_offset; curve < curves_offset + curves_count * 2U; curve += 2U) {
      vec4 p01 = texture(u_curves_texture, to_coords(curve));
      vec4 p23 = texture(u_curves_texture, to_coords(curve + 1U));

      vec2 p0 = p01.xy / v_size - position_delta;
      vec2 p1 = p01.zw / v_size - position_delta;
      vec2 p2 = p23.xy / v_size - position_delta;
      vec2 p3 = p23.zw / v_size - position_delta;

      if (max(p0.x, p3.x) * inv_pixel_size < -0.5) break;

      bool is_downwards = p0.y > 0.0 || p3.y < 0.0;

      if (
        (is_downwards && ((p0.y < 0.0 && p3.y <= 0.0) || (p0.y > 0.0 && p3.y >= 0.0))) ||
        (!is_downwards && ((p0.y <= 0.0 && p3.y < 0.0) || (p0.y >= 0.0 && p3.y > 0.0)))
      ) {
        continue;
      }

      const float epsilon = 1e-7;

      bool b01 = abs(p1.x - p0.x) + abs(p1.y - p0.y) < epsilon;
      bool b23 = abs(p3.x - p2.x) + abs(p3.y - p2.y) < epsilon;
      bool b12 = abs(p2.x - p1.x) + abs(p2.y - p1.y) < epsilon;

      vec2 delta = p3 - p0;

      float intersect = 0.0;
      float t0 = -p0.y / delta.y;

      if ((b01 && (b23 || b12)) || (b23 && b12)) {
        intersect = delta.x * t0 + p0.x;
      } else {
        vec2 a = 3.0 * p1 - 3.0 * p2 + delta;
        vec2 b = 3.0 * (p0 - 2.0 * p1 + p2);
        vec2 c = 3.0 * (p1 - p0);

        float t = calculate_cubic_root(a.y, b.y, c.y, p0.y, t0);
        float t_sq = t * t;

        intersect = a.x * t_sq * t + b.x * t_sq + c.x * t + p0.x;
      }

      coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (float(is_downwards) * 2.0 - 1.0);
    }

    return coverage;
  }

  float cubic_coverage() {
    vec2 pixel_size = vec2(fwidth(v_tex_coord.x), fwidth(v_tex_coord.y));
    
    uint curves_offset_x = (v_attr_2 >> 12) & 0xFFFU;
    uint curves_offset_y = v_attr_2 & 0xFFFU;
    uint curves_count = v_attr_1 & 0xFFFFU;
    uint curves_index_offset = curves_offset_x + curves_offset_y * 512U;
    uint winding = v_attr_1 >> 16;

    float right_coverage = float(int(winding) - 32768);
    float coverage = 0.0;
    float inv_sample_size = 1.0 / float(u_max_samples);

    // TODO: optimize sampling
    for (int y_offset = -(u_max_samples / 2); y_offset <= (u_max_samples) / 2; y_offset++) {
      vec2 sample_pos = v_tex_coord + vec2(0.0, y_offset) * pixel_size.y * inv_sample_size;
    
      coverage += cubic_horizontal_coverage(sample_pos, 1.0 / pixel_size.x, curves_index_offset, curves_count);
    }

    return right_coverage + coverage * inv_sample_size;
  }

  void main() {
    bool is_even_odd = bool((v_attr_2 >> 25) & 0x1U);
    bool is_quadratic = bool((v_attr_2 >> 24) & 0x1U);

    float alpha = v_color.a;
    float coverage = cubic_coverage();
      
    if (is_even_odd) {
      alpha = alpha * sqrt(abs(coverage - 2.0 * round(0.5 * coverage)));
    } else {
      alpha = alpha * min(abs(coverage), 1.0);
    }

    o_frag_color = vec4(v_color.rgb, 1.0) * alpha;
  }

)"
