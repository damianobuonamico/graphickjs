R"(

  precision highp float;

  uniform mat4 uViewProjection;
  uniform vec4 uColor;

  in uvec2 aPosition;
  in vec4 aPathPositionSize;
  in float aPathIndex;

  out float vPathIndex;
  out vec4 vColor;
  out vec2 vCoords;

  void main() {
    vec2 position = vec2(float(aPosition.x), float(aPosition.y));
    vec2 vertex_position = position * aPathPositionSize.zw;

    gl_Position = vec4((uViewProjection * vec4(aPathPositionSize.xy + vertex_position, 0.0, 1.0)).xyz, 1.0);
    
    vColor = uColor;
    vPathIndex = aPathIndex;
    vCoords = vertex_position + 0.5 - position;
  }

)"
