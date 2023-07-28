
R"(

  precision highp float;
  precision highp sampler2D;

  uniform mat4 uViewMatrix;
  uniform mat4 uProjectionMatrix;
  uniform ivec2 uFramebufferSize;
  uniform int uTileSize;
  uniform int uMasksTextureSize;

  in uvec2 aPosition;
  in vec4 aColor;
  in int aIndex;
  in int aMaskIndex;

  out vec4 vColor;
  out vec2 vPosition;
  out vec2 vMaskCoords;

  void main() {
    float x = float(aPosition.x);
    float y = float(aPosition.y);
    float tile = float(uTileSize);

    ivec2 tiles = ivec2(ceil(float(uFramebufferSize.x) / tile) + 2.0, ceil(float(uFramebufferSize.y) / tile) + 2.0);
    vec2 position = vec2(float((aIndex % tiles.x) * uTileSize), float((aIndex / tiles.x) * uTileSize));

    vec4 world_position = vec4(position.x + x * tile, position.y + y * tile, 0.0, 1.0);
    vec4 view_position = round(uViewMatrix * world_position);
    vec4 transformed_position = uProjectionMatrix * view_position;

    gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
    
    vColor = aColor;
    // vColor = abs(vec4(aColor.rgb, 1.0) - vec4(float(aPosition.x) * 0.1, float(aPosition.y) * 0.1, 0.0, 0.0));

    vMaskCoords = vec2(
      float(aMaskIndex % (uMasksTextureSize / uTileSize) * uTileSize) + x * (tile - 0.5) + (1.0 - x) * 0.5,
      float(aMaskIndex / (uMasksTextureSize / uTileSize) * uTileSize) + y * (tile - 0.5) + (1.0 - y) * 0.5
    ) / float(uMasksTextureSize);
  }

)"
