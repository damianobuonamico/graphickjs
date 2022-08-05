export function clamp(t: number, min: number, max: number) {
  return Math.min(Math.max(min, t), max);
}

export function round(t: number, decimals = 0) {
  decimals = 10 ** decimals;
  return Math.round((t + Number.EPSILON) * decimals) / decimals;
}

export function map(t: number, __min: number, __max: number, min: number, max: number) {
  return ((t - __min) * (max - min)) / (__max - __min) + min;
}

export function isPointInBox(point: vec2, box: Box, threshold: number = 0): boolean {
  return (
    point[0] + threshold >= box[0][0] &&
    point[0] - threshold <= box[1][0] &&
    point[1] + threshold >= box[0][1] &&
    point[1] - threshold <= box[1][1]
  );
}

export function isPointInCircle(point: vec2, center: vec2, radius: number): boolean {
  return (
    Math.pow(point[0] - center[0], 2) + Math.pow(point[1] - center[1], 2) <= Math.pow(radius, 2)
  );
}
