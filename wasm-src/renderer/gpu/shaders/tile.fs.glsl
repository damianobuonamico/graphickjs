R"(

precision mediump float;
precision mediump sampler2D;
precision mediump usampler2D;

uniform sampler2D u_curves_texture;
uniform sampler2D u_textures[${MAX_TEXTURES}];

uniform lowp int u_samples;

in lowp vec4 v_color;
in highp vec2 v_tex_coord;
in highp vec2 v_tex_coord_curves;

flat in highp uint v_attr_1;
flat in highp uint v_attr_2;
flat in highp uint v_attr_3;

out vec4 o_frag_color;

#include "cubic.glsl"

#include "texture.glsl"

void main() {
  bool is_even_odd = bool((v_attr_2 >> 9) & 0x1U);
  uint curves_type = uint((v_attr_2 >> 10) & 0x3U);
  uint paint_type = uint((v_attr_1 >> 20) & 0x7FU);

  int samples = u_samples % 2 == 0 ? u_samples + 1 : u_samples;

  // float coverage = is_quadratic ? quadratic_coverage(samples) : cubic_coverage(samples);
  float coverage = curves_type == 0U ? 1.0 : (cubic_coverage(samples));
  float alpha;

  if (is_even_odd) {
    alpha = abs(coverage - 2.0 * round(0.5 * coverage));
  } else {
    alpha = min(abs(coverage), 1.0);
  }

  if (paint_type == 3U) {
    vec4 color = texture_fill(v_attr_2 & 0x3FFU, v_tex_coord);
    o_frag_color = vec4(color.rgb, 1.0) * color.a * alpha;
  } else {
    o_frag_color = vec4(v_color.rgb, 1.0) * v_color.a * alpha + 0.000000001 * vec4(v_tex_coord_curves, 0.0, 1.0) * alpha + vec4(1.0, 0.0, 1.0, 1.0) * 0.000000000001;
  }
}

)"
