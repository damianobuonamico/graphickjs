R"(

  uniform mat4 u_mvp;

  in vec4 position;
  in vec2 texcoord;

  out vec2 v_texcoord;

  void main() {
    gl_Position = u_mvp * position;
    // gl_Position = vec4(2.0 * position.xy - 1.0, position.zw);
    v_texcoord = texcoord;
  }

)"
