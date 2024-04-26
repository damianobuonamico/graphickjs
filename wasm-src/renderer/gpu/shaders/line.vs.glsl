R"(

  precision highp float;

  uniform mat4 uViewProjection;
  uniform float uZoom;

  in vec2 aPosition;
  in vec2 aInstanceFrom;
  in vec2 aInstanceTo;
  in float aInstanceWidth;
  in uvec4 aInstanceColor;

  out vec4 vColor;
  out vec2 vTexCoord;
  out float vLineWidth;

  void main() {
    float elongation = 0.25 / uZoom;

    vec2 dir = aInstanceTo - aInstanceFrom;
    vec2 normalized_dir = normalize(dir);
    vec2 normal = aInstanceWidth * vec2(-normalized_dir.y, normalized_dir.x);
    
    vec2 position = 
      aInstanceFrom + aPosition.x * dir + 
      elongation * (2.0 * aPosition.x - 1.0) * normalized_dir + 
      normal * (1.0 - 2.0 * aPosition.y) / uZoom;

    gl_Position = vec4((uViewProjection * vec4(position, 0.0, 1.0)).xyz, 1.0);
    vColor = vec4(aInstanceColor) / 255.0;
    vTexCoord = vec2(1.0, 1.0 - 2.0 * aPosition.y);
    vLineWidth = aInstanceWidth;
  }

)"
