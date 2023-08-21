
R"(

  precision highp float;

  uniform vec2 uFramebufferSize;
  uniform lowp int uTileSize;
  uniform vec2 uOffset;

  in uvec2 aPosition;
  in vec4 aColor;
  in highp int aIndex;
  in highp ivec2 aSegmentsCoords;
  in float aZIndex;

  flat out highp ivec2 vSegmentsCoords;
  out vec4 vColor;
  out vec2 vCoords;

  void main() {
    float x = float(aPosition.x);
    float y = float(aPosition.y);
    float tile = float(uTileSize);

    ivec2 tiles = ivec2(ceil(uFramebufferSize.x / tile) + 2.0, ceil(uFramebufferSize.y / tile) + 2.0);

    vec2 pixel_size = 2.0 / uFramebufferSize;
    vec2 position = vec2(float(aIndex % tiles.x), float(aIndex / tiles.x));
    vec2 tile_size = tile * pixel_size;
    vec2 offset = round(uOffset) * pixel_size;

    gl_Position = vec4(
      (position.x + x) * tile_size.x + offset.x - 1.0,
      1.0 - (position.y + y) * tile_size.y - offset.y,
      -aZIndex,
      1.0
    );

    // vColor = aColor + vec4(x * 0.1, y * 0.1, 0.0, 0.0);
    vColor = aColor;
    vSegmentsCoords = aSegmentsCoords;
    vCoords = vec2(x, y) * float(uTileSize);

    // vMaskCoords = vec2(
    //   float(aMaskIndex % (uMasksTextureSize / uTileSize) * uTileSize) + x * (tile - 0.5) + (1.0 - x) * 0.5,
    //   float(aMaskIndex / (uMasksTextureSize / uTileSize) * uTileSize) + y * (tile - 0.5) + (1.0 - y) * 0.5
    // ) / float(uMasksTextureSize);
  }

)"
