R"(

  precision highp float;

  uniform mat4 uViewProjection;
  uniform ivec2 uFramebufferSize;
  uniform int uTileSize;

  in uvec2 aPosition;
  in uvec4 aLineSegment;
  in int aIndex;

  out vec2 vFrom;
  out vec2 vTo;
  out vec2 vPos;

  void main() {
    ivec2 tiles = uFramebufferSize / int(uTileSize);
    // ivec2 tiles = ivec2(floor(float(uFramebufferSize.x) / float(uTileSize)), floor(float(uFramebufferSize.y) / float(uTileSize)));
    vec2 position = vec2(float((aIndex % tiles.x) * uTileSize), float((aIndex / tiles.y) * uTileSize));
    vec2 fposition = vec2(aPosition);

    vec2 from = vec2(aLineSegment.x, aLineSegment.y) / 255.0 * float(uTileSize);
    vec2 to = vec2(aLineSegment.z, aLineSegment.w) / 255.0 * float(uTileSize);

    // vec2 pos = vec2(floor(min(from.x, to.x)) * (1.0 - fposition.x) + fposition.x, floor(min(from.y, to.y)) * (1.0 - fposition.y) + ceil(max(from.y, to.y)) * fposition.y);

    // vec2 pos = vec2(floor(min(from.x, to.x) * (1.0 - fposition.x)) + fposition.x * float(uTileSize), floor(min(from.y, to.y) * (1.0 - fposition.y)) + ceil(max(from.y, to.y) * fposition.y));
    // vec2 pos = vec2(min(from.x, to.x) * (1.0 - fposition.x) + fposition.x * float(uTileSize), min(from.y, to.y))


    vFrom = from;
    vTo = to;
    vPos = fposition * float(uTileSize);

    float x = position.x + fposition.x * float(uTileSize);
    float y = position.y + fposition.y * float(uTileSize);

    vec4 transformed_position = uViewProjection * vec4(x, y, 0.0, 1.0);
    gl_Position = vec4(transformed_position.xy, 0.0, 1.0);

  }

)"
