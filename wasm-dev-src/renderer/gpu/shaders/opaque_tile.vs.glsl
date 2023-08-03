R"(

  precision highp float;

  uniform mat4 uViewMatrix;
  uniform mat4 uProjectionMatrix;
  uniform ivec2 uFramebufferSize;
  uniform int uTileSize;

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
    vec2 position = vec2(float(aIndex % tiles.x), float(aIndex / tiles.x));

    vec4 temp = uProjectionMatrix * uViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);

    vec2 tile_size = 2.0 * tile / framebuffer_size;

    gl_Position = vec4(
      (position.x + x) * tile_size.x - 1.0,
      1.0 - (position.y + y) * tile_size.y,
      0.0,
      1.0
    );

    vColor = aColor * temp.x;
    vColor = vColor * 0.0000000001 + aColor;
  }

  // void main() {
  //   ivec2 tiles = ivec2(ceil(float(uFramebufferSize.x) / float(uTileSize)) + 2.0, ceil(float(uFramebufferSize.y) / float(uTileSize)) + 2.0);
  //   vec2 position = vec2(float((aIndex % tiles.x) * uTileSize), float((aIndex / tiles.x) * uTileSize));

  //   vec4 world_position = vec4(position.x + float(int(aPosition.x) * uTileSize), position.y + float(int(aPosition.y) * uTileSize), 0.0, 1.0);
  //   vec4 view_position = round(uViewMatrix * world_position);
  //   vec4 transformed_position = uProjectionMatrix * view_position;

  //   gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
    
  //   vColor = aColor;
  //   // vColor = abs(vec4(aColor.rgb, 1.0) + vec4(float(aPosition.x) * 0.1, float(aPosition.y) * 0.1, 0.0, 0.0));
  // }

)"
