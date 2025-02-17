R"(

#ifndef to_coords
#define to_coords(x) (vec2((float((x) % 256U) + 0.5) / 256.0, (float((x) / 256U) + 0.5) / 256.0))
#endif

#define QUADRATIC_EPSILON 1e-5
#define QUADRATIC_T_EPSILON 1e-4

float calculate_quadratic_root(float a, float b, float c) {
  if (abs(a) < QUADRATIC_EPSILON) {
    if (b != 0.0) return -c / b;
  }

  float discriminant = b * b - 4.0 * a * c;

  if (discriminant > -QUADRATIC_EPSILON) {
    float s = sqrt(max(0.0, discriminant));
    float t = (-b + s) / (2.0 * a);

    if (t >= -QUADRATIC_T_EPSILON && t <= 1.0 + QUADRATIC_T_EPSILON) return t;
    else return (-b - s) / (2.0 * a);
  }

  return -99999.0;
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

float quadratic_coverage(int samples) {
  vec2 pixel_size = vec2(fwidth(v_tex_coord_curves.x), fwidth(v_tex_coord_curves.y));
  float coverage = 0.0;

  uint bands = (v_attr_3 >> 24) + 1U;

  uint bands_index_offset = v_attr_3 & 0xFFFFFFU;
  uint curves_index_offset = v_attr_1 & 0xFFFFFU;

  for (int yOffset = (1 - samples) / 2; yOffset <= (samples - 1) / 2; yOffset++) {
    vec2 sample_pos = v_tex_coord_curves + vec2(0.0, yOffset) * pixel_size.y / float(samples);
  
    uint band_index = clamp(uint(floor(sample_pos.y * float(bands))), 0U, bands - 1U);
    uint band_data_start = texture(u_bands_texture, to_coords(bands_index_offset + band_index * 2U)).x + bands_index_offset;
    uint band_curves_count = texture(u_bands_texture, to_coords(bands_index_offset + band_index * 2U + 1U)).x;

    coverage += quadratic_horizontal_coverage(sample_pos, 1.0 / pixel_size.x, curves_index_offset, band_data_start, band_curves_count);
  }

  return coverage / float(samples);
}

)"
