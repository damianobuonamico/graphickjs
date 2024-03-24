R"(

  precision highp float;

  uniform mat4 uViewProjection;
  uniform vec4 uColor;
  uniform float uLineWidth;

  in vec2 aPosition;
  in vec2 aInstanceFrom;
  in vec2 aInstanceTo;

  out vec4 vColor;
  out vec2 vTexCoord;

  void main() {
    vec2 dir = aInstanceTo - aInstanceFrom;
    vec2 normalized_dir = normalize(dir);
    vec2 normal = uLineWidth * vec2(-normalized_dir.y, normalized_dir.x);
    vec2 position = aInstanceFrom + aPosition.x * dir + normal * (1.0 - 2.0 * aPosition.y);

    gl_Position = vec4((uViewProjection * vec4(position, 0.0, 1.0)).xyz, 1.0);
    vColor = uColor;
    vTexCoord = aPosition;
  }

)"
