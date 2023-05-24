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
    float tile_size = float(uTileSize);

    vec2 offset = vec2(float((aIndex % tiles.x) * uTileSize), float((aIndex / tiles.y) * uTileSize));
    vec2 vertex = vec2(aPosition);

    vec2 from = vec2(aLineSegment.x, aLineSegment.y) / 255.0 * tile_size;
    vec2 to = vec2(aLineSegment.z, aLineSegment.w) / 255.0 * tile_size;

    vec2 min_pos = floor(min(from, to));
    vec2 max_pos = ceil(max(from, to));

    vec2 position = vec2(min_pos.x * (1.0 - vertex.x) + tile_size * vertex.x, min_pos.y * (1.0 - vertex.y) + max_pos.y * vertex.y);

    vFrom = from;
    vTo = to;
    vPos = position;

    vec4 transformed_position = uViewProjection * vec4(offset + position, 0.0, 1.0);
    gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
  }

)"
