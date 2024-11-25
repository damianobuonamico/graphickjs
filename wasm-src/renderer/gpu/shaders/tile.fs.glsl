R"(
  precision mediump float;
  precision mediump sampler2D;
  precision mediump usampler2D;

  uniform usampler2D u_bands_texture;
  uniform sampler2D u_curves_texture;
  uniform sampler2D u_textures[${MAX_TEXTURES}];

  uniform lowp int u_samples;

  in lowp vec4 v_color;
  in lowp vec2 v_tex_coord;
  in highp vec2 v_tex_coord_curves;

  flat in highp uint v_attr_1;
  flat in highp uint v_attr_2;
  flat in highp uint v_attr_3;

  out vec4 o_frag_color;

  #define to_coords(x) (vec2((float((x) % 256U) + 0.5) / 256.0, (float((x) / 256U) + 0.5) / 256.0))

  float calculate_quadratic_root(float a, float b, float c) {
    const float epsilon = 1e-5;
    const float t_epsilon = 1e-4;

    if (abs(a) < epsilon) {
      if (b != 0.0) return -c / b;
    }

    float discriminant = b * b - 4.0 * a * c;

    if (discriminant > -epsilon) {
      float s = sqrt(max(0.0, discriminant));
      float t = (-b + s) / (2.0 * a);

      if (t >= -t_epsilon && t <= 1.0 + t_epsilon) return t;
      else return (-b - s) / (2.0 * a);
    }

    return -99999.0;
  }

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

  float quadratic_horizontal_coverage(vec2 pixel_pos, float inv_pixel_size, uint curves_offset, uint band_data_start, uint band_curves_count) {
    float coverage = 0.0;

    for (uint curve = 0U; curve < band_curves_count; curve++) {
      uint curve_offset = curves_offset + texture(u_bands_texture, to_coords(band_data_start + curve)).x * 2U;

      vec4 p01 = texture(u_curves_texture, to_coords(curve_offset));
      vec4 p23 = texture(u_curves_texture, to_coords(curve_offset + 1U));

      vec2 p0 = p01.xy - pixel_pos;
      vec2 p1 = p01.zw - pixel_pos;
      vec2 p2 = p23.xy - pixel_pos;

      if (max(p0.x, p2.x) * inv_pixel_size < -0.5) break;

      bool is_downwards = p0.y > 0.0 || p2.y < 0.0;

      if (
        (is_downwards && ((p0.y < 0.0 && p2.y <= 0.0) || (p0.y > 0.0 && p2.y >= 0.0))) ||
        (!is_downwards && ((p0.y <= 0.0 && p2.y < 0.0) || (p0.y >= 0.0 && p2.y > 0.0)))
      ) {
        continue;
      }

      const float epsilon = 1e-7;

      vec2 delta = p2 - p0;

      float t0 = -p0.y / delta.y;
      float intersect = delta.x * t0 + p0.x;

      if (min(p0.x, p2.x) * inv_pixel_size > 0.5) {
        coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (float(is_downwards) * 2.0 - 1.0);
        continue;
      }

      vec2 a = p0 - 2.0 * p1 + p2;
      vec2 b = 2.0 * (p1 - p0);

      float t = calculate_quadratic_root(a.y, b.y, p0.y);

      intersect = a.x * t * t + b.x * t + p0.x;

      coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (float(is_downwards) * 2.0 - 1.0);
    }

    return coverage;
  }

  float cubic_horizontal_coverage(vec2 pixel_pos, float inv_pixel_size, uint curves_offset, uint band_data_start, uint band_curves_count) {
    float coverage = 0.0;

    for (uint curve = 0U; curve < band_curves_count; curve++) {
      uint curve_offset = curves_offset + texture(u_bands_texture, to_coords(band_data_start + curve)).x * 2U;

      vec4 p01 = texture(u_curves_texture, to_coords(curve_offset));
      vec4 p23 = texture(u_curves_texture, to_coords(curve_offset + 1U));

      vec2 p0 = p01.xy - pixel_pos;
      vec2 p1 = p01.zw - pixel_pos;
      vec2 p2 = p23.xy - pixel_pos;
      vec2 p3 = p23.zw - pixel_pos;

      if (max(p0.x, p3.x) * inv_pixel_size < -0.5) break;

      bool is_downwards = p0.y > 0.0 || p3.y < 0.0;

      if (
        (is_downwards && ((p0.y < 0.0 && p3.y <= 0.0) || (p0.y > 0.0 && p3.y >= 0.0))) ||
        (!is_downwards && ((p0.y <= 0.0 && p3.y < 0.0) || (p0.y >= 0.0 && p3.y > 0.0)))
      ) {
        continue;
      }

      const float epsilon = 1e-7;

      vec2 delta = p3 - p0;

      float t0 = -p0.y / delta.y;
      float intersect = delta.x * t0 + p0.x;

      if (min(p0.x, p3.x) * inv_pixel_size > 0.5) {
        coverage += clamp(0.5 + intersect * inv_pixel_size, 0.0, 1.0) * (float(is_downwards) * 2.0 - 1.0);
        continue;
      }

      bool b01 = abs(p1.x - p0.x) + abs(p1.y - p0.y) < epsilon;
      bool b23 = abs(p3.x - p2.x) + abs(p3.y - p2.y) < epsilon;
      bool b12 = abs(p2.x - p1.x) + abs(p2.y - p1.y) < epsilon;

      if (!((b01 && (b23 || b12)) || (b23 && b12))) {
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

  float quadratic_coverage(int samples) {
    vec2 pixel_size = vec2(fwidth(v_tex_coord_curves.x), fwidth(v_tex_coord_curves.y));
    float coverage = 0.0;

    uint bands = (v_attr_3 >> 24) + 1U;

    // TODO: remove this useless offset calculation from here and CPU
    uint x_band_offset = (v_attr_3 >> 12) & 0xFFFU;
    uint y_band_offset = v_attr_3 & 0xFFFU;
    uint x_curve_offset = (v_attr_1 >> 10) & 0x3FFU;
    uint y_curve_offset = v_attr_1 & 0x3FFU;

    uint bands_index_offset = x_band_offset + y_band_offset * 256U;
    uint curves_index_offset = x_curve_offset + y_curve_offset * 256U;

    for (int yOffset = (1 - samples) / 2; yOffset <= (samples - 1) / 2; yOffset++) {
      vec2 sample_pos = v_tex_coord_curves + vec2(0.0, yOffset) * pixel_size.y / float(samples);
    
      uint band_index = clamp(uint(floor(sample_pos.y * float(bands))), 0U, bands - 1U);
      uint band_data_start = texture(u_bands_texture, to_coords(bands_index_offset + band_index * 2U)).x + bands_index_offset;
      uint band_curves_count = texture(u_bands_texture, to_coords(bands_index_offset + band_index * 2U + 1U)).x;

      coverage += quadratic_horizontal_coverage(sample_pos, 1.0 / pixel_size.x, curves_index_offset, band_data_start, band_curves_count);
    }

    return coverage / float(samples);
  }

  float cubic_coverage(int samples) {
    vec2 pixel_size = vec2(fwidth(v_tex_coord_curves.x), fwidth(v_tex_coord_curves.y));
    float coverage = 0.0;

    uint bands = (v_attr_3 >> 24) + 1U;
    
    // TODO: remove this useless offset calculation from here and CPU
    uint x_band_offset = (v_attr_3 >> 12) & 0xFFFU;
    uint y_band_offset = v_attr_3 & 0xFFFU;
    uint x_curve_offset = (v_attr_1 >> 10) & 0x3FFU;
    uint y_curve_offset = v_attr_1 & 0x3FFU;

    uint bands_index_offset = x_band_offset + y_band_offset * 256U;
    uint curves_index_offset = x_curve_offset + y_curve_offset * 256U;

    for (int yOffset = (1 - samples) / 2; yOffset <= (samples - 1) / 2; yOffset++) {
      vec2 sample_pos = v_tex_coord_curves + vec2(0.0, yOffset) * pixel_size.y / float(samples);
    
      uint band_index = clamp(uint(floor(sample_pos.y * float(bands))), 0U, bands - 1U);
      uint band_data_start = texture(u_bands_texture, to_coords(bands_index_offset + band_index * 2U)).x + bands_index_offset;
      uint band_curves_count = texture(u_bands_texture, to_coords(bands_index_offset + band_index * 2U + 1U)).x;

      coverage += cubic_horizontal_coverage(sample_pos, 1.0 / pixel_size.x, curves_index_offset, band_data_start, band_curves_count);
    }

    return coverage / float(samples);
  }

  void main() {
    bool is_even_odd = bool((v_attr_2 >> 10) & 0x1U);
    bool is_quadratic = bool((v_attr_2 >> 11) & 0x1U);
    uint paint_type = uint((v_attr_1 >> 20) & 0x7FU);

    int samples = u_samples % 2 == 0 ? u_samples + 1 : u_samples;

    float coverage = is_quadratic ? quadratic_coverage(samples) : cubic_coverage(samples);
    float alpha = v_color.a;

    if (is_even_odd) {
      alpha = alpha * abs(coverage - 2.0 * round(0.5 * coverage));
    } else {
      alpha = alpha * min(abs(coverage), 1.0);
    }

    // TODO: shader includes
    if (paint_type == 3U) {
      uint paint_coord = v_attr_2 & 0x3FFU;

       // TODO: more than these 3 textures should be supported in CPU shader precompilation
      switch (paint_coord) {
      case 1U:
        o_frag_color = texture(u_textures[1], v_tex_coord) * alpha;
        return;
      case 2U:
        o_frag_color = texture(u_textures[2], v_tex_coord) * alpha;
        return;
      case 0U:
      default:
        o_frag_color = vec4(0.0, 0.0, 0.0, 1.0) * alpha;
        return;
      }
    } else {
      // o_frag_color = vec4(v_color.rgb, 1.0) * alpha;
      o_frag_color = vec4(v_color.rgb, 1.0) * alpha + vec4(v_tex_coord_curves, 0.0, 0.0) * 0.1;
    }
  }

)"
