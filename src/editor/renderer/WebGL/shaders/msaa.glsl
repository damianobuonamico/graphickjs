#vertex

attribute vec2 aPosition;

void main() {
  gl_Position = vec4(aPosition.xy, 0.0, 1.0);
}

#fragment

precision mediump float;

uniform sampler2D uScreenTexture;
uniform vec2 uResolution;

void main() {
  
  // Texture coordinates

  vec2 fragCoord = gl_FragCoord.xy;
  vec2 inverseVP = 1.0 / uResolution;
  vec2 v_rgbM = vec2(fragCoord * inverseVP);
  vec4 texColor = texture2D(uScreenTexture, v_rgbM);
  
  // Output 
  
  gl_FragColor = texColor;
}