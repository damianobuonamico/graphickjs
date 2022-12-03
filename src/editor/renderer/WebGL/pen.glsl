#vertex

uniform mat3 uViewProjectionMatrix;

attribute vec2 aPosition;

varying vec4 vColor;

void main() {
  gl_Position = vec4((uViewProjectionMatrix * vec3(aPosition, 1)).xy, 0.0, 1.0);
  vColor = vec4(0.0, 0.0, 0.0, 0.5);
}

#fragment

precision mediump float;

varying highp vec4 vColor;

void main() {
  gl_FragColor = vColor;
}