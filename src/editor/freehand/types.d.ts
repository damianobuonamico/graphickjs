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

interface FreehandGeometryOptions {
  size?: number;
  thinning?: number;
  smoothing?: number;
  streamline?: number;
  taperStart?: number;
  taperEnd?: number;
  simulatePressure?: boolean;
}

interface StrokePoint {
  point: vec2;
  pressure: number;
  vector: vec2;
  distance: number;
  runningLength: number;
}

interface StrokeLastPoint {
  p0: { index: number; penIndex: number; point: vec2 };
  p1: { index: number; penIndex: number; point: vec2 };
  angle: number;
}

interface PenPointDistance {
  index: number;
  distance: number;
}

interface StrokeJoint {
  leftPenIndex: number;
  rightPenIndex: number;
  center: vec2;
  left: vec2;
  right: vec2;
  prevLeft: vec2;
  prevRight: vec2;
}

interface StrokeParsedJoint {
  leftIndex: number;
  rightIndex: number;
  prevLeftIndex: number;
  prevRightIndex: number;
  collapsedLeft: boolean;
  collapsedRight: boolean;
}

interface FreehandStrokeOptions {
  width?: number;
  enableLineSmoothing?: boolean;
}
