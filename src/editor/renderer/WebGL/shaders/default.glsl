#vertex

uniform mat4 uTransformMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec4 uColor;

attribute vec2 aPosition;

varying vec4 vColor;

void main() {
  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * uTransformMatrix * vec4(aPosition, 0.0, 1.0);
  vColor = uColor;
}

#fragment

precision mediump float;

varying highp vec4 vColor;

void main() {
  gl_FragColor = vColor;
}
