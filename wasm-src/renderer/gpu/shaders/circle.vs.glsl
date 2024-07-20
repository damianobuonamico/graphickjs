R"(

  precision highp float;

  uniform mat4 uViewProjection;

  in uvec2 aPosition;
  in vec2 aInstancePosition;
  in float aInstanceRadius;
  in uvec4 aInstanceColor;

  out vec4 vColor;
  out vec2 vTexCoord;
  out float vRadius;

  void main() {
    vec2 position = vec2(aPosition);

    gl_Position = vec4((uViewProjection * vec4(aInstancePosition + 2.0 * position * aInstanceRadius - aInstanceRadius, 0.0, 1.0)).xyz, 1.0);
    
    vColor = vec4(aInstanceColor) / 255.0;
    vTexCoord = position - 0.5;
    vRadius = aInstanceRadius;
  }

)"
