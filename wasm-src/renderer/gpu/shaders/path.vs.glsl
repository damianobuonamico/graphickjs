R"(

  precision highp float;
  precision highp int;

  uniform mat4 uViewProjection;
  uniform vec2 uViewportSize;

  in vec2 aPosition;
  in vec4 aInstanceAttrib1;
  in vec2 aInstanceAttrib2;
  in vec2 aInstancePosition;
  in vec2 aInstanceSize;
  in uvec4 aInstanceColor;
  in uint aInstanceCurvesData;
  in uint aInstanceBandsData;

  out vec4 vColor;
  out vec2 vTexCoord;

  flat out vec2 vPosition;
  flat out vec2 vSize;
  flat out uint vCurvesData;
  flat out uint vBandsData;

  void main() {
    mat3 transform = mat3(
      aInstanceSize.x,     0.0,                 0.0,
      0.0,                 aInstanceSize.y,     0.0,
      aInstancePosition.x, aInstancePosition.y, 1.0
    );

    mat4 model = mat4(
      aInstanceAttrib1.x, aInstanceAttrib1.w, 0.0, 0.0,
      aInstanceAttrib1.y, aInstanceAttrib2.x, 0.0, 0.0,
      aInstanceAttrib1.z, aInstanceAttrib2.y, 1.0, 0.0,
      0.0,                0.0,                0.0, 1.0
    );

    mat4 m = uViewProjection * model;

    vec2 p = (transform * vec3(aPosition, 1.0)).xy;
    vec2 n = normalize(aPosition - 0.5);
    
    float w = uViewportSize.x;
    float h = uViewportSize.y;
    float s = m[0][3] * p.x + m[1][3] * p.y + m[3][3];
    float t = m[0][3] * n.x + m[1][3] * n.y;
    float u = w * (s * (m[0][0] * n.x + m[1][0] * n.y) - t * (m[0][0] * p.x + m[1][0] * p.y + m[3][0]));
    float v = h * (s * (m[0][1] * n.x + m[1][1] * n.y) - t * (m[0][1] * p.x + m[1][1] * p.y + m[3][1]));

    float d = s * s * (s * t + sqrt(u * u + v * v)) / (u * u + v * v - s * s * t * t)  * 2.0;

    // vec2 xDilate = (model * transform * vec3(1.0, 0.0, 1.0)).xy;
    // vec2 yDilate = (model * transform * vec3(0.0, 1.0, 1.0)).xy;
    // vec2 origin = (model * transform * vec3(0.0, 0.0, 1.0)).xy;

    // float xDist = distance(xDilate, origin) * uZoom / 5.0;
    // float yDist = distance(yDilate, origin) * uZoom / 5.0;

    // gl_Position = uViewProjection * (vec4(model * transform * vec3(aPosition + vec2(aPosition - 0.5) / vec2(xDist, yDist), 1.0), 1.0));
    gl_Position = m * vec4(p + n * d, 1.0, 1.0);

    vColor = vec4(float(aInstanceColor.x) / 255.0, float(aInstanceColor.y) / 255.0 + uViewportSize.x * 0.00000000000000001, float(aInstanceColor.z) / 255.0, float(aInstanceColor.w) / 255.0);
    vTexCoord = aPosition + n * d / aInstanceSize;
    // vTexCoord = aPosition;
    // vTexCoord = aPosition + (aPosition - 0.5) / vec2(xDist, yDist);

    vPosition = aInstancePosition;
    vSize = aInstanceSize;
    vCurvesData = aInstanceCurvesData;
    vBandsData = aInstanceBandsData;
  }

)"
