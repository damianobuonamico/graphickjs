R"(

precision mediump float;
precision mediump sampler2D;

uniform sampler2D u_textures[${MAX_TEXTURES}];

in lowp vec4 v_color;
in lowp vec2 v_tex_coord;

flat in highp uint v_attr_1;
flat in highp uint v_attr_2;

out vec4 o_frag_color;

#include "texture.glsl"

void main() {
  int paint_type = int((v_attr_1 >> 20) & 0x7FU);

  if (paint_type == 3) {
    vec4 color = texture_fill(v_attr_2 & 0x3FFU, v_tex_coord);
    o_frag_color = vec4(color.rgb, 1.0) * color.a;
  } else {
    o_frag_color = vec4(v_color.rgb, 1.0) * v_color.a;
  }
}

)"
