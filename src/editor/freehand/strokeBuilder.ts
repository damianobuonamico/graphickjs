import { vec2 } from '@/math';

export function getFreehandStrokeGeometry(
  points: vec3[],
  zoom: number,
  { width = 8, enableLineSmoothing = true }: FreehandStrokeOptions
): [Float32Array, number[]] {
  width /= 2;

  if (enableLineSmoothing) {
    const smoothPoints = smoothFreehandPoints(points);
    return getLinesGeometry(smoothPoints, width);
  } else {
    return getLinesGeometry(points, width);
  }
}

function getLinesGeometry(points: vec3[], size: number): [Float32Array, number[]] {
  const vertices: number[] = [];
  const indices: number[] = [];
  let offset = 0;
  let connectingLine = false; // TODO: Check what really does
  let finishingLine = true; // TODO: Check what really does
  let overdraw = 0.5; // TODO: Check what really does

  if (points.length < 1) return [Float32Array.from(vertices), indices];

  let prevPoint: vec2 = [points[0][0], points[0][1]];
  let prevWidth = points[0][2] * size;
  let prevC: vec2 = [0, 0];
  let prevD: vec2 = [0, 0];
  let prevG: vec2 = [0, 0];
  let prevI: vec2 = [0, 0];

  const temp: vec2 = [0, 0];

  for (let i = 1, n = points.length; i < n; ++i) {
    const point: vec2 = [points[i][0], points[i][1]];
    const width = points[i][2] * size;

    if (vec2.equals(point, prevPoint, 0.0001)) continue;

    const direction = vec2.sub(point, prevPoint);
    const perp = vec2.perp(direction);

    vec2.normalize(perp, perp);

    vec2.mulS(perp, prevWidth, temp);

    let A = vec2.add(prevPoint, temp);
    let B = vec2.sub(prevPoint, temp);

    vec2.mulS(perp, width, temp);

    const C = vec2.add(point, temp);
    const D = vec2.sub(point, temp);

    if (connectingLine || indices.length > 0) {
      A = prevC;
      B = prevD;
    } else if (indices.length === 0) {
      // circles.push_back(CirclePoint {curPoint.pos, curPoint.width, (linePoints[i - 1].pos - curPoint.pos).getNormalized()});
    }

    triangulateRect(A, B, C, D, vertices, indices);

    prevC = C;
    prevD = D;

    if (finishingLine && i === n - 1) {
      // circles.push_back(CirclePoint {curPoint.pos, curPoint.width, (curPoint.pos - linePoints[i - 1].pos).getNormalized()});
      finishingLine = false;
    }

    prevPoint = point;
    prevWidth = width;

    vec2.mulS(perp, overdraw, temp);

    let F = vec2.add(A, temp);
    const G = vec2.add(C, temp);
    let H = vec2.sub(B, temp);
    const I = vec2.sub(D, temp);

    if (connectingLine || indices.length > 6) {
      F = prevG;
      H = prevI;
    }

    prevG = G;
    prevI = I;

    // triangulateRect(F, fadeOutColor, A, color, G, fadeOutColor, C, color, _vertices, _indices, 0);
    // triangulateRect(B, color, H, fadeOutColor, D, color, I, fadeOutColor, _vertices, _indices, 0);
  }

  // for (auto c : circles) {
  //   triangulateCircle(c, color, Overdraw, _vertices, _indices);
  // }

  if (indices.length > 0) {
    connectingLine = true;
  }

  return [Float32Array.from(vertices), indices];
}

function triangulateRect(
  A: vec2,
  B: vec2,
  C: vec2,
  D: vec2,
  vertices: number[],
  indices: number[]
) {
  const startIndex = vertices.length / 2;

  vertices.push(...A, ...B, ...C, ...D);
  indices.push(
    startIndex,
    startIndex + 1,
    startIndex + 2,
    startIndex + 1,
    startIndex + 2,
    startIndex + 3
  );
}

export function getPointsGeometry(points: vec3[] | vec2[], size: number = 0.2): [Float32Array, number[]] {
  const vertices: number[] = [];
  const indices: number[] = [];

  let offset = 0;

  for (const point of points) {
    const width = (point[2] || 1) * size;

    vertices.push(
      point[0] - width,
      point[1] - width,
      point[0] + width,
      point[1] - width,
      point[0] + width,
      point[1] + width,
      point[0] - width,
      point[1] + width
    );
    indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);

    offset += 4;
  }

  return [Float32Array.from(vertices), indices];
}

function smoothFreehandPoints(points: vec3[]) {
  const result: vec3[] = [];
  if (points.length > 2) {
    for (let i = 2, n = points.length; i < n; ++i) {
      const prev2 = points[i - 2];
      const prev1 = points[i - 1];
      const point = points[i];

      const midPoint1 = vec2.mid(<any>prev1, <any>prev2);
      const midPoint2 = vec2.mid(<any>point, <any>prev1);

      const segmentDistance = 2;
      const distance = vec2.dist(midPoint1, midPoint2);
      const numberOfSegments = Math.min(128, Math.max(Math.floor(distance / segmentDistance), 32));

      let t = 0;
      const step = 1 / numberOfSegments;

      for (let j = 0; j < numberOfSegments; ++j) {
        const a = Math.pow(1 - t, 2);
        const b = (1 - t) * t;
        const c = t * t;

        result.push([
          midPoint1[0] * a + prev1[0] * 2 * b + midPoint2[0] * c,
          midPoint1[1] * a + prev1[1] * 2 * b + midPoint2[1] * c,
          a * ((prev1[2] + prev2[2]) * 0.5) + 2 * b * prev1[2] + c * ((point[2] + prev1[2]) * 0.5)
        ]);

        t += step;
      }

      result.push([...midPoint2, (point[2] + prev1[2]) * 0.5]);
    }
  }
  return result;
}
