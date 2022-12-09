#vertex

attribute vec2 aPosition;

void main() {
  gl_Position = vec4(aPosition.xy, 0.0, 1.0);
}

#fragment

precision mediump float;

uniform sampler2D uScreenTexture;
uniform vec2 uResolution;

#ifndef FXAA_REDUCE_MIN
  #define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
  #define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
  #define FXAA_SPAN_MAX     8.0
#endif

// TODO: Reimplement to support alpha
void main() {
  
  // Texture coordinates

  vec2 fragCoord = gl_FragCoord.xy;

  vec2 inverseVP = 1.0 / uResolution;
  vec2 v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
	vec2 v_rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
	vec2 v_rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
	vec2 v_rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
  vec2 v_rgbM = vec2(fragCoord * inverseVP);

  // FXAA

  vec4 color;

  vec3 rgbNW = texture2D(uScreenTexture, v_rgbNW).xyz;
  vec3 rgbNE = texture2D(uScreenTexture, v_rgbNE).xyz;
  vec3 rgbSW = texture2D(uScreenTexture, v_rgbSW).xyz;
  vec3 rgbSE = texture2D(uScreenTexture, v_rgbSE).xyz;
  vec4 texColor = texture2D(uScreenTexture, v_rgbM);
  vec3 rgbM  = texColor.xyz;
  vec3 luma = vec3(0.299, 0.587, 0.114);
  float lumaNW = dot(rgbNW, luma);
  float lumaNE = dot(rgbNE, luma);
  float lumaSW = dot(rgbSW, luma);
  float lumaSE = dot(rgbSE, luma);
  float lumaM  = dot(rgbM,  luma);
  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
  
  mediump vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
  
  float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                        (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
  
  float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
  dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
            max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
            dir * rcpDirMin)) * inverseVP;
  
  vec3 rgbA = 0.5 * (
    texture2D(uScreenTexture, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
    texture2D(uScreenTexture, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
  vec3 rgbB = rgbA * 0.5 + 0.25 * (
    texture2D(uScreenTexture, fragCoord * inverseVP + dir * -0.5).xyz +
    texture2D(uScreenTexture, fragCoord * inverseVP + dir * 0.5).xyz);

  float lumaB = dot(rgbB, luma);
  if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
    color = vec4(rgbA, texColor.a);
  }
  else {
    color = vec4(rgbB, texColor.a);
  }
  
  // Output 
  
  gl_FragColor = color;
}