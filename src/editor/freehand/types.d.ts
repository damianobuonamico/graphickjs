interface Sample {
  position: vec2;
  weightedPosition: vec2;
  distance: number;
  duration: number;
  time: number;
}

interface WobbleSmootherParams {
  timeout: number;
  speedFloor: number;
  speedCeiling: number;
}
