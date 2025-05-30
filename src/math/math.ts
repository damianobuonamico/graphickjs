import { MATH_EPSILON, MATH_TWO_PI } from '@/utils/constants';
import { mat2, vec2 } from '.';

export function average(a: number, b: number) {
  return (a + b) * 0.5;
}

export function lerp(a: number, b: number, t: number) {
  return a + t * (b - a);
}

export function normalizeAngle(angle: number): number {
  while (angle < 0) angle += MATH_TWO_PI;
  while (angle > MATH_TWO_PI) angle -= MATH_TWO_PI;
  return angle;
}

export function lerpAngle(start: number, end: number, interp_amount: number): number {
  start = normalizeAngle(start);
  end = normalizeAngle(end);

  const delta = end - start;

  if (delta < -Math.PI) end += MATH_TWO_PI;
  else if (delta > Math.PI) end -= MATH_TWO_PI;

  return normalizeAngle(lerp(start, end, interp_amount));
}

export function equals(a: number, b: number) {
  return Math.abs(a - b) <= MATH_EPSILON * Math.max(1, Math.abs(a), Math.abs(b));
}

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

export function snap(t: number, intervals: number = 8) {
  const increment = (Math.PI * 2) / intervals;
  return Math.round(t / increment) * increment;
}

export function getLinesFromBox(box: Box): Box[] {
  return [
    [box[0], [box[1][0], box[0][1]]],
    [[box[1][0], box[0][1]], box[1]],
    [box[1], [box[0][0], box[1][1]]],
    [[box[0][0], box[1][1]], box[0]]
  ];
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

export function doesLineIntersectLine(a: Box, b: Box) {
  const [[c, d], [j, k]] = a;
  const [[p, q], [r, s]] = b;

  let det, gamma, lambda;

  det = (j - c) * (s - q) - (r - p) * (k - d);

  if (det === 0) return false;

  lambda = ((s - q) * (r - c) + (p - r) * (s - d)) / det;
  gamma = ((d - k) * (r - c) + (j - c) * (s - d)) / det;

  return 0 < lambda && lambda < 1 && 0 < gamma && gamma < 1;
}

export function doesBoxIntersectBox(a: Box, b: Box): boolean {
  return b[1][0] >= a[0][0] && a[1][0] >= b[0][0] && b[1][1] >= a[0][1] && a[1][1] >= b[0][1];
}

export function doesBoxIntersectRotatedBox(a: Box, b: Box, angle: number): boolean {
  function overlaps(axis: [vec2, vec2], origin: vec2, b: vec2[]) {
    for (let i = 0; i < 2; ++i) {
      let t = vec2.dot(b[0], axis[i]);
      let tMin = t;
      let tMax = t;

      for (let c = 1; c < 4; ++c) {
        t = vec2.dot(b[c], axis[i]);

        if (t < tMin) tMin = t;
        else if (t > tMax) tMax = t;
      }

      if (tMin > 1 + origin[i] || tMax < origin[i]) return false;
    }

    return true;
  }

  const center = vec2.mid(b[0], b[1]);

  const vertices: vec2[] = [a[0], [a[1][0], a[0][1]], a[1], [a[0][0], a[1][1]]];
  const rotated = [
    vec2.rotate(b[0], center, angle),
    vec2.rotate([b[1][0], b[0][1]], center, angle),
    vec2.rotate(b[1], center, angle),
    vec2.rotate([b[0][0], b[1][1]], center, angle)
  ];

  const axis: [vec2, vec2] = [
    vec2.sub(vertices[1], vertices[0]),
    vec2.sub(vertices[3], vertices[0])
  ];
  const rotatedAxis: [vec2, vec2] = [
    vec2.sub(rotated[1], rotated[0]),
    vec2.sub(rotated[3], rotated[0])
  ];

  const origin = vec2.create();
  const rotatedOrigin = vec2.create();

  for (let i = 0; i < 2; ++i) {
    vec2.divS(axis[i], vec2.sqrLen(axis[i]), axis[i]);
    origin[i] = vec2.dot(vertices[0], axis[i]);

    vec2.divS(rotatedAxis[i], vec2.sqrLen(rotatedAxis[i]), rotatedAxis[i]);
    rotatedOrigin[i] = vec2.dot(rotated[0], rotatedAxis[i]);
  }

  return overlaps(axis, origin, rotated) && overlaps(rotatedAxis, rotatedOrigin, vertices);
}

export function closestPointToLine(point: vec2, line: Box): [vec2, number] {
  const [a, b] = vec2.sub(point, line[0]);
  const [c, d] = vec2.sub(line[1], line[0]);

  const dot = a * c + b * d;
  const lenSquare = c * c + d * d;

  let param = -1;

  if (lenSquare !== 0) param = dot / lenSquare;

  let vec: vec2;
  if (param < 0) vec = line[0];
  else if (param > 1) vec = line[1];
  else vec = vec2.add(line[0], [param * c, param * d]);

  return [vec, vec2.dist(point, vec)];
}

export function closestPointToBox(point: vec2, box: Box): [vec2, number] {
  const lines = getLinesFromBox(box);

  let closest: [vec2, number] = [vec2.create(), Infinity];

  for (const line of lines) {
    const distance = closestPointToLine(point, line);
    if (distance[1] < closest[1]) closest = distance;
  }

  return closest;
}

export function getLineCircleIntersections(
  line: Box,
  center: vec2,
  radius: number
): { point: vec2; t: number }[] {
  const v1 = vec2.sub(line[1], line[0]);
  const v2 = vec2.sub(line[0], center);

  const b = -2 * vec2.dot(v1, v2);
  const c = 2 * vec2.sqrLen(v1);
  const d = Math.sqrt(b * b - 2 * c * (vec2.sqrLen(v2) - radius * radius));

  if (isNaN(d)) return [];

  const u1 = (b - d) / c;
  const u2 = (b + d) / c;

  const ret: { point: vec2; t: number }[] = [];

  if (u1 <= 1 && u1 >= 0) ret.push({ point: vec2.add(line[0], vec2.mulS(v1, u1)), t: u1 });
  if (u2 <= 1 && u2 >= 0) ret.push({ point: vec2.add(line[0], vec2.mulS(v1, u2)), t: u2 });

  return ret;
}

export function getLineLineIntersection([p0, p1]: Box, [p2, p3]: Box): vec2 | null {
  const s10_x = p1[0] - p0[0],
    s10_y = p1[1] - p0[1],
    s32_x = p3[0] - p2[0],
    s32_y = p3[1] - p2[1];
  const denom = s10_x * s32_y - s32_x * s10_y;
  if (denom == 0) return null; // collinear
  const s02_x = p0[0] - p2[0],
    s02_y = p0[1] - p2[1];
  const s_numer = s10_x * s02_y - s10_y * s02_x;
  if (s_numer < 0 == denom > 0) return null; // no collision
  const t_numer = s32_x * s02_y - s32_y * s02_x;
  if (t_numer < 0 == denom > 0) return null; // no collision
  if (s_numer > denom == denom > 0 || t_numer > denom == denom > 0) return null; // no collision
  // collision detected
  const t = t_numer / denom;
  return [p0[0] + t * s10_x, p0[1] + t * s10_y];
  // const v1 = vec2.sub(b1, a1);
  // const v2 = vec2.sub(b2, a2);

  // const A: mat2 = [...v1, -v2[0], -v2[1]];
  // mat2.inv(A);

  // const B: vec2 = [a2[0] - a1[0], a2[1] - a1[1]];
  // mat2.mulVec2(A, B, B);

  // if (B[0] >= 0 && B[0] <= 1 && B[1] >= 0 && B[1] <= 1) vec2.add(a1, vec2.mulS(v1, B[0], v1), v1);
  // return null;
}

export function pointLineSquaredDistance(p: vec2, [a, b]: Box) {
  const d = vec2.sqrDist(a, b);
  if (d === 0) return vec2.sqrDist(p, a);
  const t = ((p[0] - a[0]) * (b[0] - a[0]) + (p[1] - a[1]) * (b[1] - a[1])) / d;
  return vec2.sqrDist(p, [a[0] + t * (b[0] - a[0]), a[1] + t * (b[1] - a[1])]);
}
