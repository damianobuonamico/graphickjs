import { clamp, vec2, vec3 } from '@/math';
import { MATH_TWO_PI } from '@/utils/constants';
import earcut from 'earcut';

const drawPoints = false;

const RATE_OF_PRESSURE_CHANGE = 0.275;
const FIXED_PI = Math.PI + 0.0001;

function getStrokeRadius(
  size: number,
  thinning: number,
  pressure: number,
  easing: (t: number) => number = (t) => t
) {
  return size * easing(0.5 - thinning * (0.5 - pressure));
}

function simplifyRadialDist(points: vec3[], sqTolerance: number) {
  let prevPoint = points[0];
  let point = prevPoint;

  const newPoints: vec3[] = [prevPoint];

  for (let i = 1, len = points.length; i < len; i++) {
    point = points[i];

    if (vec2.sqrDist(<any>point, <any>prevPoint) > sqTolerance) {
      newPoints.push(point);
      prevPoint = point;
    }
  }

  if (prevPoint !== point) newPoints.push(point);

  return newPoints;
}

function getSqSegDist(p: vec3, p1: vec3, p2: vec3) {
  let x = p1[0],
    y = p1[1],
    dx = p2[0] - x,
    dy = p2[1] - y;

  if (dx !== 0 || dy !== 0) {
    var t = ((p[0] - x) * dx + (p[1] - y) * dy) / (dx * dx + dy * dy);

    if (t > 1) {
      x = p2[0];
      y = p2[1];
    } else if (t > 0) {
      x += dx * t;
      y += dy * t;
    }
  }

  dx = p[0] - x;
  dy = p[1] - y;

  return dx * dx + dy * dy;
}

function simplifyDPStep(
  points: vec3[],
  first: number,
  last: number,
  sqTolerance: number,
  simplified: vec3[]
) {
  let maxSqDist = sqTolerance;
  let index = 0;

  for (let i = first + 1; i < last; i++) {
    var sqDist = getSqSegDist(points[i], points[first], points[last]);

    if (sqDist > maxSqDist) {
      index = i;
      maxSqDist = sqDist;
    }
  }

  if (maxSqDist > sqTolerance) {
    if (index - first > 1) simplifyDPStep(points, first, index, sqTolerance, simplified);
    simplified.push(points[index]);
    if (last - index > 1) simplifyDPStep(points, index, last, sqTolerance, simplified);
  }
}

function simplifyDouglasPeucker(points: vec3[], sqTolerance: number) {
  let last = points.length - 1;

  const simplified = [points[0]];

  simplifyDPStep(points, 0, last, sqTolerance, simplified);
  simplified.push(points[last]);

  return simplified;
}

export function simplify(points: vec3[], tolerance: number) {
  if (points.length <= 2) return points;
  const sqTolerance = tolerance * tolerance;

  return simplifyDouglasPeucker(simplifyRadialDist(points, sqTolerance), sqTolerance);
}

function generateStrokingPen(center: vec2, radius: number, sides: number) {
  const points: vec2[] = [];
  const increment = MATH_TWO_PI / sides;
  const vector: vec2 = [radius, 0];
  const origin: vec2 = [0, 0];

  for (let i = 0; i < sides; i++) {
    const point = vec2.rotate(vector, origin, i * increment);
    points.push(vec2.add(point, center, point));
  }

  return points;
}

const debugOffset = false;

export function getFreehandGeometrye(
  inputPoints: vec3[],
  zoom: number,
  {
    size = 8,
    thinning = 0.6,
    smoothing = 0.5,
    streamline = 0.1,
    taperStart = 0,
    taperEnd = 0,
    simulatePressure = false
  }: FreehandGeometryOptions
): [Float32Array, number[]] {
  const positions: number[] = [];
  const indices: number[] = [];

  const points = simplify(inputPoints, 2);

  let offset = 0;
  let lastPen: vec2[];
  const penResolution = 8;

  if (points.length < 2) return [Float32Array.from(positions), indices];

  const lastIndices = [0, 0];

  {
    const point: vec2 = <any>points[0];
    const next: vec2 = <any>points[1];
    const w = points[0][2] * size;

    const v = vec2.sub(next, point);
    const na = vec2.perp(v);

    vec2.normalize(na, na);
    vec2.mulS(na, w, na);

    const pen = generateStrokingPen(<any>point, w * size, penResolution);

    let min1 = [0, Infinity];
    let min2 = [0, Infinity];

    for (let j = 0; j < pen.length; ++j) {
      const P = pen[j];
      const v = vec2.sub(point, next);

      const cx = P[0] - point[0];
      const cy = P[1] - point[1];

      const d = v[0] * v[1];

      const ux = cx * (1 - v[0] * v[0]) - cy * d;
      const uy = cy * (1 - v[1] * v[1]) - cx * d;

      const distance = ux * ux + uy * uy;

      if (distance < min1[1]) {
        min2[0] = min1[0];
        min2[1] = min1[1];
        min1[0] = j;
        min1[1] = distance;
      } else if (distance < min2[1]) {
        min2[0] = j;
        min2[1] = distance;
      }
    }

    lastIndices[0] = min1[0];
    lastIndices[1] = min2[0];

    for (const p of [point, vec2.add(na, point), vec2.add(vec2.neg(na), point)]) {
      positions.push(
        p[0] - 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] + 0.2,
        p[0] - 0.2,
        p[1] + 0.2
      );
      indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
      offset += 4;
    }

    const res = 20;
    for (let t = 0; t < res; ++t) {
      const p = vec2.lerp(point, next, t / res);
      positions.push(
        p[0] - 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] + 0.2,
        p[0] - 0.2,
        p[1] + 0.2
      );
      indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
      offset += 4;
    }
  }

  for (let i = 1, n = points.length - 1; i < n; ++i) {
    const point = points[i];
    const pen = generateStrokingPen(<any>point, point[2] * size, penResolution);

    let min1 = [0, Infinity];
    let min2 = [0, Infinity];

    for (let j = 0; j < pen.length; ++j) {
      // const vertex = pen[j];
      // const lastVertex = lastPen[j];

      const P = pen[j];
      const v = vec2.sub(<any>point, <any>points[i + 1]);

      const cx = P[0] - point[0];
      const cy = P[1] - point[1];

      const d = v[0] * v[1];

      const ux = cx * (1 - v[0] * v[0]) - cy * d;
      const uy = cy * (1 - v[1] * v[1]) - cx * d;

      const distance = ux * ux + uy * uy;

      if (distance < min1[1]) {
        min2[0] = min1[0];
        min2[1] = min1[1];
        min1[0] = j;
        min1[1] = distance;
      } else if (distance < min2[1]) {
        min2[0] = j;
        min2[1] = distance;
      }

      // for (let t = 0; t < res; ++t) {
      //   const p = vec2.lerp(vertex, lastVertex, t / res);
      //   positions.push(
      //     p[0] - 0.2,
      //     p[1] - 0.2,
      //     p[0] + 0.2,
      //     p[1] - 0.2,
      //     p[0] + 0.2,
      //     p[1] + 0.2,
      //     p[0] - 0.2,
      //     p[1] + 0.2
      //   );
      //   indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
      //   offset += 4;
      // }
    }

    const res = 20;
    for (let t = 0; t < res; ++t) {
      const p = vec3.lerp(point, points[i + 1], t / res);
      positions.push(
        p[0] - 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] + 0.2,
        p[0] - 0.2,
        p[1] + 0.2
      );
      indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
      offset += 4;
    }

    for (const po of [pen[lastIndices[0]], pen[lastIndices[1]], pen[min1[0]], pen[min2[0]]]) {
      positions.push(
        po[0] - 0.2,
        po[1] - 0.2,
        po[0] + 0.2,
        po[1] - 0.2,
        po[0] + 0.2,
        po[1] + 0.2,
        po[0] - 0.2,
        po[1] + 0.2
      );
      indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
      offset += 4;
    }

    lastIndices[0] = min1[0];
    lastIndices[1] = min2[0];

    // lastPen = pen;
  }

  {
    const point: vec2 = <any>points[points.length - 1];
    const prev: vec2 = <any>points[points.length - 2];
    const w = points[points.length - 1][2] * size;

    const v = vec2.sub(prev, point);
    const na = vec2.perp(v);

    vec2.normalize(na, na);
    vec2.mulS(na, w, na);

    for (const p of [point, vec2.add(vec2.neg(na), point), vec2.add(na, point)]) {
      positions.push(
        p[0] - 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] - 0.2,
        p[0] + 0.2,
        p[1] + 0.2,
        p[0] - 0.2,
        p[1] + 0.2
      );
      indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
      offset += 4;
    }
  }

  return [Float32Array.from(positions), indices];
}

export function getFreehandGeometry2(
  inputPoints: vec3[],
  zoom: number,
  {
    size = 8,
    thinning = 0.6,
    smoothing = 0.5,
    streamline = 0.1,
    taperStart = 0,
    taperEnd = 0,
    simulatePressure = false
  }: FreehandGeometryOptions
): [Float32Array, number[]] {
  const positions: number[] = [];
  const left: number[] = [];
  const right: number[] = [];
  const indices: number[] = [];

  const points = simplify(inputPoints, 0 / zoom);

  if (points.length < 2) return [new Float32Array(0), []];

  let offset = 0;

  {
    const a: vec2 = <any>points[0];
    const b: vec2 = <any>points[1];
    const w = size * points[0][2];

    const va = vec2.sub(b, a);
    const na = vec2.perp(va);

    vec2.normalize(na, na);
    vec2.mulS(na, w, na);

    const nc = vec2.neg(na);

    vec2.add(na, a, na);
    vec2.add(nc, a, nc);

    if (debugOffset) {
      for (const point of [a, na, nc]) {
        positions.push(
          point[0] - 0.2,
          point[1] - 0.2,
          point[0] + 0.2,
          point[1] - 0.2,
          point[0] + 0.2,
          point[1] + 0.2,
          point[0] - 0.2,
          point[1] + 0.2
        );
        indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
        offset += 4;
      }
    } else {
      // positions.push(...na, ...nc);
      left.push(...na);
      right.push(...nc);
      offset += 2;
    }
  }

  for (let i = 1; i < points.length - 1; ++i) {
    const a: vec2 = <any>points[i - 1];
    const b: vec2 = <any>points[i];
    const c: vec2 = <any>points[i + 1];
    const w = size * points[i][2];

    const va = vec2.sub(b, a);
    const vc = vec2.sub(b, c);

    const na = vec2.perp(va);
    const nc = vec2.perp(vc);

    vec2.normalize(na, na);
    vec2.normalize(nc, nc);

    vec2.mulS(na, w, na);
    vec2.mulS(nc, w, nc);

    vec2.add(na, b, na);
    vec2.add(nc, b, nc);

    if (debugOffset) {
      for (const point of [b, na, nc]) {
        positions.push(
          point[0] - 0.2,
          point[1] - 0.2,
          point[0] + 0.2,
          point[1] - 0.2,
          point[0] + 0.2,
          point[1] + 0.2,
          point[0] - 0.2,
          point[1] + 0.2
        );
        indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
        offset += 4;
      }
    } else {
      // positions.push(...na, ...nc);
      // indices.push(offset - 2, offset - 1, offset, offset - 1, offset, offset + 1);
      left.push(...na);
      right.push(...nc);
      offset += 2;
    }
  }

  {
    const a: vec2 = <any>points[points.length - 1];
    const b: vec2 = <any>points[points.length - 2];
    const w = size * points[points.length - 1][2];

    const va = vec2.sub(b, a);
    const na = vec2.perp(va);

    vec2.normalize(na, na);
    vec2.mulS(na, w, na);

    const nc = vec2.neg(na);

    vec2.add(na, a, na);
    vec2.add(nc, a, nc);

    if (debugOffset) {
      for (const point of [a, nc, na]) {
        positions.push(
          point[0] - 0.2,
          point[1] - 0.2,
          point[0] + 0.2,
          point[1] - 0.2,
          point[0] + 0.2,
          point[1] + 0.2,
          point[0] - 0.2,
          point[1] + 0.2
        );
        indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
        offset += 4;
      }
    } else {
      // positions.push(...nc, ...na);
      // indices.push(offset - 2, offset - 1, offset, offset - 1, offset, offset + 1);
      left.push(...nc);
      right.push(...na);
      offset += 2;
    }
  }

  if (!debugOffset) {
    for (let i = right.length - 1; i >= 0; i -= 2) {
      left.push(right[i - 1], right[i]);
    }

    return [Float32Array.from(left), earcut(left)];
  }

  return [Float32Array.from(positions), indices];
}

export function getFreehandGeometryd(
  points: vec3[],
  zoom: number,
  {
    size = 8,
    thinning = 0.6,
    smoothing = 0.5,
    streamline = 0.1,
    taperStart = 0,
    taperEnd = 0,
    simulatePressure = false
  }: FreehandGeometryOptions
): [Float32Array, number[]] {
  // Filter points based on distance and current zoom level

  if (points.length === 0) return [new Float32Array(0), []];

  const positions: number[] = [];
  const indices: number[] = [];

  const t = 0.15 + (1 - streamline) * 0.85;

  if (points.length === 2) {
    const last = points[1];
    points = points.slice(0, -1);

    for (let i = 1; i < 5; i++) {
      points.push(vec3.lerp(points[0], last, i / 4));
    }
  }

  if (points.length === 1) {
    points = [...points, vec3.add(points[0], [1, 1, 0])];
  }

  const strokePoints: StrokePoint[] = [
    {
      point: [points[0][0], points[0][1]],
      pressure: points[0][2] >= 0 ? points[0][2] : 0.25,
      vector: [1, 1],
      distance: 0,
      runningLength: 0
    }
  ];

  let runningLength = 0;
  let prev = strokePoints[0];

  // TODO: adjust weight
  const max = points.length - 1;
  const min = 2 / zoom;

  for (let i = 1; i < max; ++i) {
    const point = vec2.lerp(prev.point, [points[i][0], points[i][1]], t);
    if (vec2.equals(prev.point, point)) continue;

    const distance = vec2.dist(point, prev.point);
    runningLength += distance;

    if (distance < min) continue;

    // TODO: optimize double square root calculations
    prev = {
      point,
      pressure: points[i][2],
      vector: vec2.normalize(vec2.sub(prev.point, point)),
      distance,
      runningLength
    };

    strokePoints.push(prev);
  }

  {
    const point: vec2 = [points[max][0], points[max][1]];
    const distance = vec2.dist(point, prev.point);
    runningLength += distance;

    strokePoints.push({
      point,
      pressure: points[max][2],
      vector: vec2.normalize(vec2.sub(prev.point, point)),
      distance: distance,
      runningLength
    });
  }

  strokePoints[0].vector = strokePoints[1]?.vector || [0, 0];

  let offset = 0;

  if (drawPoints) {
    for (let i = 0; i < strokePoints.length; ++i) {
      const point = strokePoints[i].point;

      positions.push(
        point[0] - 1,
        point[1] - 1,
        point[0] + 1,
        point[1] - 1,
        point[0] + 1,
        point[1] + 1,
        point[0] - 1,
        point[1] + 1
      );
      indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
      offset += 4;
    }
  }

  // Calculate outline points
  // const minDistance = Math.pow(size * smoothing, 2);
  // const totalLength = runningLength;
  // const easing = (t: number) => t;
  // const taperStartEase = (t: number) => t * (2 - t);
  // const taperEndEase = (t: number) => --t * t * t + 1;

  // const leftPts: number[][] = [];
  // const rightPts: number[][] = [];

  // // Average first five points' pressure values
  // let prevPressure = strokePoints.slice(0, 10).reduce((acc, curr) => {
  //   let pressure = curr.pressure;

  //   if (simulatePressure) {
  //     // Speed of change - how fast should the the pressure changing?
  //     const sp = Math.min(1, curr.distance / size);
  //     // Rate of change - how much of a change is there?
  //     const rp = Math.min(1, 1 - sp);
  //     // Accelerate the pressure
  //     pressure = Math.min(1, acc + (rp - acc) * (sp * RATE_OF_PRESSURE_CHANGE));
  //   }

  //   return (acc + pressure) / 2;
  // }, strokePoints[0].pressure);

  // let radius = getStrokeRadius(
  //   size,
  //   thinning,
  //   strokePoints[strokePoints.length - 1].pressure,
  //   easing
  // );

  // let firstRadius: number | undefined = undefined;
  // let prevVector = strokePoints[0].vector;
  // let pl = strokePoints[0].point;
  // let pr = pl;
  // let isPrevPointSharpCorner = false;

  // let tl = pl;
  // let tr = pr;

  // for (let i = 0; i < strokePoints.length; i++) {
  //   let { pressure } = strokePoints[i];
  //   const { point, vector, distance, runningLength } = strokePoints[i];

  //   // Removes noise from the end of the line
  //   if (i < strokePoints.length - 1 && totalLength - runningLength < 3) {
  //     continue;
  //   }

  //   if (thinning) {
  //     if (simulatePressure) {
  //       const sp = Math.min(1, distance / size);
  //       const rp = Math.min(1, 1 - sp);
  //       pressure = Math.min(1, prevPressure + (rp - prevPressure) * (sp * RATE_OF_PRESSURE_CHANGE));
  //     }

  //     radius = getStrokeRadius(size, thinning, pressure, easing);
  //   } else {
  //     radius = size / 2;
  //   }

  //   if (firstRadius === undefined) {
  //     firstRadius = radius;
  //   }

  //   /*
  //     Apply tapering
  //     If the current length is within the taper distance at either the
  //     start or the end, calculate the taper strengths. Apply the smaller
  //     of the two taper strengths to the radius.
  //   */

  //   const ts = runningLength < taperStart ? taperStartEase(runningLength / taperStart) : 1;

  //   const te =
  //     totalLength - runningLength < taperEnd
  //       ? taperEndEase((totalLength - runningLength) / taperEnd)
  //       : 1;

  //   radius = Math.max(0.01, radius * Math.min(ts, te));

  //   /* Add strokePoints to left and right */

  //   /*
  //     Handle sharp corners
  //     Find the difference (dot product) between the current and next vector.
  //     If the next vector is at more than a right angle to the current vector,
  //     draw a cap at the current point.
  //   */

  //   const nextVector = (i < strokePoints.length - 1 ? strokePoints[i + 1] : strokePoints[i]).vector;
  //   const nextDpr = i < strokePoints.length - 1 ? vec2.dot(vector, nextVector) : 1.0;
  //   const prevDpr = vec2.dot(vector, prevVector);

  //   const isPointSharpCorner = prevDpr < 0 && !isPrevPointSharpCorner;
  //   const isNextPointSharpCorner = nextDpr !== null && nextDpr < 0;

  //   if (isPointSharpCorner || isNextPointSharpCorner) {
  //     // It's a sharp corner. Draw a rounded cap and move on to the next point
  //     // Considering saving these and drawing them later? So that we can avoid
  //     // crossing future strokePoints.

  //     const offset = vec2.mulS(vec2.perp(prevVector), radius);

  //     for (let step = 1 / 13, t = 0; t <= 1; t += step) {
  //       tl = vec2.rotate(vec2.sub(point, offset), point, FIXED_PI * t);
  //       leftPts.push(tl);

  //       tr = vec2.rotate(vec2.add(point, offset), point, FIXED_PI * -t);
  //       rightPts.push(tr);
  //     }

  //     pl = tl;
  //     pr = tr;

  //     if (isNextPointSharpCorner) {
  //       isPrevPointSharpCorner = true;
  //     }
  //     continue;
  //   }

  //   isPrevPointSharpCorner = false;

  //   // Handle the last point
  //   if (i === strokePoints.length - 1) {
  //     const offset = vec2.mulS(vec2.perp(vector), radius);
  //     leftPts.push(vec2.sub(point, offset));
  //     rightPts.push(vec2.add(point, offset));
  //     continue;
  //   }

  //   /*
  //     Add regular strokePoints
  //     Project strokePoints to either side of the current point, using the
  //     calculated size as a distance. If a point's distance to the
  //     previous point on that side greater than the minimum distance
  //     (or if the corner is kinda sharp), add the strokePoints to the side's
  //     strokePoints array.
  //   */

  //   const offset = vec2.mulS(vec2.perp(vec2.lerp(nextVector, vector, nextDpr)), radius);

  //   tl = vec2.sub(point, offset);

  //   if (i <= 1 || vec2.sqrDist(pl, tl) > minDistance) {
  //     leftPts.push(tl);
  //     pl = tl;
  //   }

  //   tr = vec2.add(point, offset);

  //   if (i <= 1 || vec2.sqrDist(pr, tr) > minDistance) {
  //     rightPts.push(tr);
  //     pr = tr;
  //   }

  //   // Set variables for next iteration
  //   prevPressure = pressure;
  //   prevVector = vector;
  // }

  // /*
  //   Drawing caps

  //   Now that we have our strokePoints on either side of the line, we need to
  //   draw caps at the start and end. Tapered lines don't have caps, but
  //   may have dots for very short lines.
  // */

  // const firstPoint = strokePoints[0].point.slice(0, 2);

  // const lastPoint =
  //   strokePoints.length > 1
  //     ? strokePoints[strokePoints.length - 1].point.slice(0, 2)
  //     : vec2.add(strokePoints[0].point, [1, 1]);

  // const startCap: number[][] = [];

  // const endCap: number[][] = [];

  // /*
  //   Draw a dot for very short or completed strokes

  //   If the line is too short to gather left or right strokePoints and if the line is
  //   not tapered on either side, draw a dot. If the line is tapered, then only
  //   draw a dot if the line is both very short and complete. If we draw a dot,
  //   we can just return those strokePoints.
  // */

  // if (strokePoints.length === 1) {
  //   // if (!(taperStart || taperEnd) || isComplete) {
  //   const start = vec2.proj(
  //     <vec2>firstPoint,
  //     vec2.normalize(vec2.perp(vec2.sub(<vec2>firstPoint, <vec2>lastPoint))),
  //     -(firstRadius || radius)
  //   );
  //   const dotPts: number[][] = [];
  //   for (let step = 1 / 13, t = step; t <= 1; t += step) {
  //     dotPts.push(vec2.rotate(start, <vec2>firstPoint, FIXED_PI * 2 * t));
  //   }
  //   // }
  // } else {
  //   /*
  //   Draw a start cap
  //   Unless the line has a tapered start, or unless the line has a tapered end
  //   and the line is very short, draw a start cap around the first point. Use
  //   the distance between the second left and right point for the cap's radius.
  //   Finally remove the first left and right strokePoints. :psyduck:
  // */

  //   if (taperStart || (taperEnd && strokePoints.length === 1)) {
  //     // The start point is tapered, noop
  //   } else {
  //     // Draw the round cap - add thirteen strokePoints rotating the right point around the start point to the left point
  //     for (let step = 1 / 13, t = step; t <= 1; t += step) {
  //       const pt = vec2.rotate(<vec2>rightPts[0], <vec2>firstPoint, FIXED_PI * t);
  //       startCap.push(pt);
  //     }
  //   }

  //   /*
  //   Draw an end cap
  //   If the line does not have a tapered end, and unless the line has a tapered
  //   start and the line is very short, draw a cap around the last point. Finally,
  //   remove the last left and right strokePoints. Otherwise, add the last point. Note
  //   that This cap is a full-turn-and-a-half: this prevents incorrect caps on
  //   sharp end turns.
  // */

  //   const direction = vec2.perp(vec2.neg(strokePoints[strokePoints.length - 1].vector));

  //   if (taperEnd || (taperStart && strokePoints.length === 1)) {
  //     // Tapered end - push the last point to the line
  //     endCap.push(lastPoint);
  //   } else {
  //     // Draw the round end cap
  //     const start = vec2.proj(<vec2>lastPoint, direction, radius);
  //     for (let step = 1 / 29, t = step; t < 1; t += step) {
  //       endCap.push(vec2.rotate(start, <vec2>lastPoint, FIXED_PI * 3 * t));
  //     }
  //   }
  // }

  // const outline = leftPts.concat(endCap, rightPts.reverse(), startCap);

  // if (!drawPoints) {
  //   for (let i = 0; i < outline.length; ++i) {
  //     const point = <vec2>outline[i];

  //     positions.push(
  //       point[0] - 1,
  //       point[1] - 1,
  //       point[0] + 1,
  //       point[1] - 1,
  //       point[0] + 1,
  //       point[1] + 1,
  //       point[0] - 1,
  //       point[1] + 1
  //     );
  //     indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
  //     offset += 4;
  //   }
  // }

  {
    const point = strokePoints[0].point;
    const next = strokePoints[1].point;
    const width = strokePoints[0].pressure * size;

    const v = vec2.sub(next, point);

    vec2.normalize(v, v);
    vec2.mulS(v, width, v);

    const na: vec2 = [-v[1], v[0]];
    const nc: vec2 = [v[1], -v[0]];

    positions.push(...vec2.add(na, point, na), ...vec2.add(nc, point, nc));

    offset += 2;
  }

  for (let i = 1; i < strokePoints.length; ++i) {
    const prev = strokePoints[i - 1].point;
    const point = strokePoints[i].point;
    const width = strokePoints[i].pressure * size;

    const v = vec2.sub(point, prev);

    vec2.normalize(v, v);
    vec2.mulS(v, width, v);

    const na: vec2 = [-v[1], v[0]];
    const nc: vec2 = [v[1], -v[0]];

    positions.push(...vec2.add(na, point, na), ...vec2.add(nc, point, nc));
    indices.push(offset - 2, offset - 1, offset, offset - 1, offset, offset + 1);

    offset += 2;
  }

  return [Float32Array.from(positions), indices];
}

function sqr(x: number) {
  return x * x;
}

function dist2(v: vec2, w: vec2) {
  return sqr(v[0] - w[0]) + sqr(v[1] - w[1]);
}

// p - point
// v - start point of segment
// w - end point of segment
function distToSegmentSquared(p: vec2, v: vec2, w: vec2) {
  var l2 = dist2(v, w);
  if (l2 === 0) return dist2(p, v);
  var t = ((p[0] - v[0]) * (w[0] - v[0]) + (p[1] - v[1]) * (w[1] - v[1])) / l2;
  // t = Math.max(0, Math.min(1, t));
  return dist2(p, [v[0] + t * (w[0] - v[0]), v[1] + t * (w[1] - v[1])]);
}

function updateMaxDistance(i: number, P: vec2, a: vec2, v: vec2, max1: vec2, max2: vec2) {
  // const cx = P[0] - a[0];
  // const cy = P[1] - a[1];

  // const d = v[0] * v[1];

  // const ux = cx * (1 - v[0] * v[0]) - cy * d;
  // const uy = cy * (1 - v[1] * v[1]) - cx * d;

  // const distance = ux * ux + uy * uy;

  const distance = distToSegmentSquared(P, a, v);

  if (distance > max1[1]) {
    max2[0] = max1[0];
    max2[1] = max1[1];
    max1[0] = i;
    max1[1] = distance;
  } else if (distance > max2[1]) {
    max2[0] = i;
    max2[1] = distance;
  }
}

function pushPoints(positions: number[], indices: number[], offset: number, points: vec2[]) {
  for (const point of points) {
    positions.push(
      point[0] - 0.2,
      point[1] - 0.2,
      point[0] + 0.2,
      point[1] - 0.2,
      point[0] + 0.2,
      point[1] + 0.2,
      point[0] - 0.2,
      point[1] + 0.2
    );
    indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);
    offset += 4;
  }
  return offset;
}

function pushLerpPoints(positions: number[], indices: number[], offset: number, a: vec2, b: vec2) {
  const res = Math.round(vec2.dist(a, b));
  const points = [];

  for (let t = 0; t < res; ++t) {
    points.push(vec2.lerp(a, b, t / res));
  }

  return pushPoints(positions, indices, offset, points);
}

function shouldSwapCouples(a: vec2, b1: vec2, b2: vec2, realSlope: number) {
  const m1 = (b1[1] - a[1]) / (b1[0] - a[0]);
  const m2 = (b2[1] - a[1]) / (b2[0] - a[0]);

  if (realSlope > 999) return Math.abs(m1) > Math.abs(m2);

  return Math.abs(m1 - realSlope) < Math.abs(m2 - realSlope);
}

const debug = false;

export function getFreehandGeometry(
  inputPoints: vec3[],
  zoom: number,
  {
    size = 8,
    thinning = 0.6,
    smoothing = 0.5,
    streamline = 0.1,
    taperStart = 0,
    taperEnd = 0,
    simulatePressure = false
  }: FreehandGeometryOptions
): [Float32Array, number[]] {
  const positions: number[] = [];
  const indices: number[] = [];

  const points = simplify(inputPoints, 0 / zoom);

  let offset = 0;
  const penResolution = 12;

  if (points.length < 2) return [Float32Array.from(positions), indices];

  console.time('geo');

  const lastIndices: vec2 = [0, 0];
  const lastPoints: [vec2, vec2] = [
    [0, 0],
    [0, 0]
  ];
  let lastSlope = 0;

  const pens: vec2[][] = [];

  {
    const point: vec2 = <any>points[0];
    const next: vec2 = <any>points[1];
    const w = points[0][2] * size;

    const v = vec2.sub(next, point);
    const na = vec2.perp(v);

    vec2.normalize(na, na);
    vec2.mulS(na, w, na);

    const pen = generateStrokingPen(point, w, penResolution);

    let max1: vec2 = [0, -Infinity];
    let max2: vec2 = [0, -Infinity];

    for (let j = 0; j < pen.length; ++j) {
      updateMaxDistance(j, pen[j], point, next /*vec2.sub(point, next)*/, max1, max2);
    }

    const a = vec2.add(na, point);
    const b = vec2.add(vec2.neg(na, na), point);

    if (debug) {
      offset = pushPoints(positions, indices, offset, [
        point,
        vec2.add(na, point),
        vec2.add(vec2.neg(na), point)
      ]);

      offset = pushLerpPoints(positions, indices, offset, point, next);
    } else {
      positions.push(...a, ...b);
      offset += 2;
    }

    lastIndices[0] = max1[0];
    lastIndices[1] = max2[0];
    lastPoints[0] = a;
    lastPoints[1] = b;
    lastSlope = (next[1] - point[1]) / (next[0] - point[0]);
  }

  for (let i = 1, n = points.length - 1; i < n; ++i) {
    const point: vec2 = <any>points[i];
    const next: vec2 = <any>points[i + 1];
    const pen = generateStrokingPen(point, points[i][2] * size, penResolution);

    let max1: vec2 = [0, -Infinity];
    let max2: vec2 = [0, -Infinity];

    for (let j = 0; j < pen.length; ++j) {
      updateMaxDistance(j, pen[j], point, next /*vec2.sub(point, next)*/, max1, max2);
    }

    if (debug) {
      offset = pushPoints(positions, indices, offset, [
        pen[lastIndices[0]],
        pen[lastIndices[1]],
        pen[max1[0]],
        pen[max2[0]]
      ]);

      offset = pushLerpPoints(positions, indices, offset, point, next);

      if (shouldSwapCouples(lastPoints[0], pen[lastIndices[0]], pen[lastIndices[1]], lastSlope)) {
        offset = pushLerpPoints(positions, indices, offset, lastPoints[0], pen[lastIndices[0]]);
        offset = pushLerpPoints(positions, indices, offset, lastPoints[1], pen[lastIndices[1]]);
      } else {
        offset = pushLerpPoints(positions, indices, offset, lastPoints[0], pen[lastIndices[1]]);
        offset = pushLerpPoints(positions, indices, offset, lastPoints[1], pen[lastIndices[0]]);
      }
    } else {
      positions.push(
        ...pen[lastIndices[0]],
        ...pen[lastIndices[1]],
        ...pen[max1[0]],
        ...pen[max2[0]]
      );
      if (shouldSwapCouples(lastPoints[0], pen[lastIndices[0]], pen[lastIndices[1]], lastSlope)) {
        indices.push(offset - 2, offset - 1, offset, offset - 1, offset, offset + 1);
      }
      offset += 4;
    }

    lastPoints[0] = pen[max1[0]];
    lastPoints[1] = pen[max2[0]];
    lastIndices[0] = max1[0];
    lastIndices[1] = max2[0];
    lastSlope = (next[1] - point[1]) / (next[0] - point[0]);
    pens.push(pen);
  }

  {
    const point: vec2 = <any>points[points.length - 1];
    const prev: vec2 = <any>points[points.length - 2];
    const w = points[points.length - 1][2] * size;

    const v = vec2.sub(prev, point);
    const na = vec2.perp(v);

    vec2.normalize(na, na);
    vec2.mulS(na, w, na);

    const a = vec2.add(na, point);
    const b = vec2.add(vec2.neg(na), point);

    if (debug) {
      offset = pushPoints(positions, indices, offset, [point, a, b]);

      if (shouldSwapCouples(lastPoints[0], a, b, lastSlope)) {
        offset = pushLerpPoints(positions, indices, offset, lastPoints[0], a);
        offset = pushLerpPoints(positions, indices, offset, lastPoints[1], b);
      } else {
        offset = pushLerpPoints(positions, indices, offset, lastPoints[0], b);
        offset = pushLerpPoints(positions, indices, offset, lastPoints[1], a);
      }
    } else {
      positions.push(...a, ...b);
      if (shouldSwapCouples(lastPoints[0], a, b, lastSlope)) {
        indices.push(offset - 2, offset - 1, offset, offset - 1, offset, offset + 1);
      }
      offset += 2;
    }
  }

  for (let i = 0, n = pens.length; i < n; ++i) {
    const center = offset;
    const pen = pens[i];

    positions.push(points[i + 1][0], points[i + 1][1], ...pen[0]);
    offset += 2;

    for (let j = 1, m = pen.length; j < m; ++j) {
      positions.push(...pen[j]);
      indices.push(center, offset - 1, offset);
      offset++;
    }

    indices.push(center, offset - 1, center + 1);
  }
  console.timeEnd('geo');

  return [Float32Array.from(positions), indices];
}
