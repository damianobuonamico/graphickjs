R"(

  precision highp float;

  uniform ivec2 uFramebufferSize;
  uniform int uTileSize;
  uniform vec2 uOffset;

  in uvec2 aPosition;
  in vec4 aColor;
  in int aIndex;

  out vec4 vColor;

  void main() {
    float x = float(aPosition.x);
    float y = float(aPosition.y);
    float tile = float(uTileSize);
    vec2 framebuffer_size = vec2(float(uFramebufferSize.x), float(uFramebufferSize.y));

    ivec2 tiles = ivec2(ceil(framebuffer_size.x / tile) + 2.0, ceil(framebuffer_size.y / tile) + 2.0);
   
    vec2 pixel_size = 2.0 / framebuffer_size;
    vec2 position = vec2(float(aIndex % tiles.x), float(aIndex / tiles.x));
    vec2 tile_size = tile * pixel_size;
    vec2 offset = round(uOffset) * pixel_size;

    gl_Position = vec4(
      (position.x + x) * tile_size.x + offset.x - 1.0,
      1.0 - (position.y + y) * tile_size.y - offset.y,
      0.0,
      1.0
    );

    vColor = aColor;
  }

)"
