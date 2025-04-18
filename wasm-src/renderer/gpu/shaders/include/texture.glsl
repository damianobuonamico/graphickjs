R"(

vec4 texture_fill(uint texture_index, vec2 tex_coord) {
  switch (texture_index) {
  ${TEXTURE_CASES}
  case 0U:
  default:
    return vec4(0.0, 0.0, 0.0, 1.0);
  }
}

)"
