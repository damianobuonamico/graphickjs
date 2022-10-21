import { MATH_EPSILON } from '@/utils/constants';
import { vec2 } from '.';

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
