
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

  #define MASKS_TEXTURE_SIZE (16 * 100)

  void main() {
    ivec2 tiles = ivec2(ceil(float(uFramebufferSize.x) / float(uTileSize)) + 2.0, ceil(float(uFramebufferSize.y) / float(uTileSize)) + 2.0);
    vec2 position = vec2(float((aIndex % tiles.x) * uTileSize), float((aIndex / tiles.x) * uTileSize));

    vec4 world_position = vec4(position.x + float(int(aPosition.x) * uTileSize), position.y + float(int(aPosition.y) * uTileSize), 0.0, 1.0);
    vec4 view_position = round(uViewMatrix * world_position);
    vec4 transformed_position = uProjectionMatrix * view_position;

    gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
    
    // vColor = aColor;
    vColor = abs(vec4(aColor.rgb, 1.0) - vec4(float(aPosition.x) * 0.1, float(aPosition.y) * 0.1, 0.0, 0.0));
    vMaskCoords = vec2(
      (float(aMaskIndex % (MASKS_TEXTURE_SIZE / uTileSize) * uTileSize + int(aPosition.x) * uTileSize) + 0.3) / float(MASKS_TEXTURE_SIZE),
      (float(aMaskIndex / (MASKS_TEXTURE_SIZE / uTileSize) * uTileSize + int(aPosition.y) * uTileSize)) / float(MASKS_TEXTURE_SIZE)
    );
  }

)"
