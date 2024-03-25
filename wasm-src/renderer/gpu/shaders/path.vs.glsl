R"(

  precision highp float;
  precision highp int;

  uniform mat4 uViewProjection;

  in vec2 aPosition;
  in vec4 aAttrib1;
  in vec2 aAttrib2;
  in uvec4 aColor;

  out vec4 vColor;

  void main() {
    mat4 model = mat4(
      aAttrib1.x, aAttrib1.w, 0.0, 0.0,
      aAttrib1.y, aAttrib2.x, 0.0, 0.0,
      aAttrib1.z, aAttrib2.y, 1.0, 0.0,
      0.0,       0.0,         0.0, 1.0
    );

    gl_Position = uViewProjection * model * vec4(aPosition, 0.0, 1.0);

    vColor = vec4(aColor.x / 255.0, aColor.y / 255.0, aColor.z / 255.0, aColor.w / 255.0);
  }

)"
