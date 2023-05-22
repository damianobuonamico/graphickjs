R"(

  precision highp float;
  precision highp sampler2D;

  uniform mat4 uViewProjection;
  uniform ivec2 uFramebufferSize;
  uniform int uTileSize;
  uniform sampler2D uMaskTexture;

  in uvec2 aPosition;
  in vec4 aColor;
  in int aIndex;
  in int aMaskIndex;

  out vec4 vColor;
  out vec2 vMaskTexCoord;

  void main() {
    ivec2 tiles = ivec2(ceil(float(uFramebufferSize.x) / float(uTileSize)) + 2.0, ceil(float(uFramebufferSize.y) / float(uTileSize)) + 2.0);
    vec2 position = vec2(float((aIndex % tiles.x) * uTileSize), float((aIndex / tiles.x) * uTileSize));

    vec4 transformed_position = uViewProjection * vec4(floor(position.x + float(int(aPosition.x) * uTileSize)), floor(position.y + float(int(aPosition.y) * uTileSize)), 0.0, 1.0);
    gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
    
    // vColor = aColor - vec4(float(aPosition.x) * 0.2, float(aPosition.y) * 0.2, 0.0, 0.0);

    // ivec2 textiles = ivec2(ceil(float(uFramebufferSize.x) / float(uTileSize)), ceil(float(uFramebufferSize.y) / float(uTileSize)));
    // vec2 texcoords = vec2(float((aMaskIndex % textiles.x) * uTileSize), float((aMaskIndex / textiles.x) * uTileSize));
    // vec2 texcoords = vec2(0.0 + float(aMaskIndex) * 0.00000000000000000001, 0.0);
    vColor = aColor;

    int tex_tiles = max(uFramebufferSize.x, uFramebufferSize.y) / uTileSize;
    // ivec2 tex_tiles = ivec2(floor(float(uFramebufferSize.x) / float(uTileSize)), floor(float(uFramebufferSize.y) / float(uTileSize)));
    // vec2 tex_coords = vec2((float((aMaskIndex % tex_tiles) * int(uTileSize)) + 0.5) / float(uFramebufferSize), 1.0 - (float((aMaskIndex / tex_tiles) * int(uTileSize)) + 0.5) / float(uFramebufferSize));

    // vec2 temp = vec2(aPosition) * vec2(uTileSize) / float(max(uFramebufferSize.x, uFramebufferSize.y));
    // vMaskTexCoord = tex_coords + temp;

    float texture_size = float(uTileSize) * float(tex_tiles);
    vec2 tex_coords_origin = vec2(float(aMaskIndex % tex_tiles) * float(uTileSize), float(aMaskIndex / tex_tiles) * float(uTileSize)) / texture_size;
    vec2 tex_coords_size = vec2(ivec2(aPosition) * uTileSize) / texture_size;

    vMaskTexCoord = vec2(tex_coords_origin.x + tex_coords_size.x, 1.0 - tex_coords_origin.y - tex_coords_size.y);
  }

)"
