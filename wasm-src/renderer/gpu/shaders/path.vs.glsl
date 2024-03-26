R"(

  precision highp float;
  precision highp int;

  uniform mat4 uViewProjection;

  in vec2 aPosition;
  in vec4 aInstanceAttrib1;
  in vec2 aInstanceAttrib2;
  in vec2 aInstancePosition;
  in vec2 aInstanceSize;
  in uvec4 aInstanceColor;

  out vec4 vColor;
  out vec2 vTexCoord;

  flat out vec2 vPosition;
  flat out vec2 vSize;

  void main() {
    mat3 transform = mat3(
      aInstanceSize.x, 0.0, 0.0,
      0.0, aInstanceSize.y, 0.0,
      aInstancePosition.x, aInstancePosition.y, 1.0
    );

    mat3 model = mat3(
      aInstanceAttrib1.x, aInstanceAttrib1.w, 0.0,
      aInstanceAttrib1.y, aInstanceAttrib2.x, 0.0,
      aInstanceAttrib1.z, aInstanceAttrib2.y, 1.0
    );

    gl_Position = uViewProjection * (vec4(model * transform * vec3(aPosition, 1.0), 1.0) + vec4(aPosition - 0.5, 0.0, 0.0));

    vColor = vec4(aInstanceColor.x / 255.0, aInstanceColor.y / 255.0, aInstanceColor.z / 255.0, aInstanceColor.w / 255.0);
    vTexCoord = aPosition + (aPosition - 0.5) / aInstanceSize;

    vPosition = aInstancePosition;
    vSize = aInstanceSize;
  }

)"
