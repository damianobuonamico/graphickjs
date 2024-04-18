R"(

  precision highp float;

  uniform mat4 uViewProjection;

  in vec2 aPosition;
  in vec2 aInstancePosition;
  in float aInstanceRadius;
  in uvec4 aInstanceColor;

  out vec4 vColor;
  out vec2 vTexCoord;
  out float vRadius;

  void main() {
    gl_Position = vec4((uViewProjection * vec4(aInstancePosition + 2.0 * aPosition * aInstanceRadius - aInstanceRadius, 0.0, 1.0)).xyz, 1.0);
    
    vColor = vec4(aInstanceColor) / 255.0;
    vTexCoord = aPosition - 0.5;
    vRadius = aInstanceRadius;
  }

)"
