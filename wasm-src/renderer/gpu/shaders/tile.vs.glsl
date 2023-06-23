R"(

  precision highp float;
  precision highp sampler2D;

  uniform mat4 uViewMatrix;
  uniform mat4 uProjectionMatrix;
  uniform ivec2 uFramebufferSize;
  uniform int uTileSize;

  in uvec2 aPosition;
  in vec4 aColor;
  in int aIndex;
  in int aMaskIndex;

  out vec4 vColor;
  out vec2 vPosition;
  out vec2 vMaskCoords;

  flat out int vSegmentIndex;

  #define TILE_OVERLAP 0
  #define SEGMENTS_TEXTURE_SIZE 2048

  void main() {
    ivec2 tiles = ivec2(ceil(float(uFramebufferSize.x) / float(uTileSize)) + 2.0, ceil(float(uFramebufferSize.y) / float(uTileSize)) + 2.0);
    vec2 position = vec2(float((aIndex % tiles.x) * uTileSize), float((aIndex / tiles.x) * uTileSize));

    vec4 transformed_position = uProjectionMatrix * round(uViewMatrix * vec4(floor(position.x + float(int(aPosition.x) * uTileSize)), floor(position.y + float(int(aPosition.y) * uTileSize)), 0.0, 1.0));
    gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
    
    // vColor = aColor;
    vColor = abs(vec4(aColor.rgb, 1.0) - vec4(float(aPosition.x) * 0.2, float(aPosition.y) * 0.2, 0.0, 0.0));

    vPosition = vec2(aPosition) * float(uTileSize) + float(TILE_OVERLAP);
    vSegmentIndex = aMaskIndex;
    vMaskCoords = vec2(
      float(aMaskIndex % (SEGMENTS_TEXTURE_SIZE / uTileSize) * uTileSize + int(aPosition.x) * uTileSize) / float(SEGMENTS_TEXTURE_SIZE),
      float(aMaskIndex / (SEGMENTS_TEXTURE_SIZE / uTileSize) * uTileSize + int(aPosition.y) * uTileSize) / float(SEGMENTS_TEXTURE_SIZE)
    );
  }

)"
