R"(

  precision highp float;

  uniform mat4 uViewProjection;

  in uvec2 aPosition;
  in vec2 aPathPosition;
  in vec2 aPathSize;
  in float aSegmentsIndex;
  in float aColorIndex;

  out float vPathIndex;
  out vec4 vColor;
  out vec2 vCoords;
  out vec2 vSize;

  void main() {
    vec2 position = vec2(float(aPosition.x), float(aPosition.y));
    vec2 vertex_position = position * aPathSize;

    gl_Position = vec4((uViewProjection * vec4(aPathPosition + vertex_position, 0.0, 1.0)).xyz, 1.0);
    
    vColor = vec4(aColorIndex * 0.0000001, 0.0, 0.0, 1.0);
    vPathIndex = aSegmentsIndex;
    vCoords = vertex_position;
  }

)"
