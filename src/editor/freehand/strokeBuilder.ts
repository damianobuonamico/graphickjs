import { equals, getLineLineIntersection, pointLineSquaredDistance, vec2 } from '@/math';
import { GEOMETRY_CURVE_ERROR, MATH_TWO_PI } from '@/utils/constants';

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

function simplify(points: vec3[], tolerance: number) {
  if (points.length <= 2) return points;
  const sqTolerance = tolerance * tolerance;

  return simplifyDouglasPeucker(simplifyRadialDist(points, sqTolerance), sqTolerance);
}

function circle(
  positions: number[],
  indices: number[],
  offset: number,
  center: vec2,
  radius: number,
  zoom: number
) {
  let increment = 2 * Math.acos(1 - GEOMETRY_CURVE_ERROR / (radius * zoom));
  const sides = Math.max(Math.round(MATH_TWO_PI / increment), 3);
  increment = MATH_TWO_PI / sides;

  const vector: vec2 = [center[0] + radius, center[1]];
  const index = offset;

  positions.push(...center, ...vector);
  offset += 2;

  for (let i = 1; i < sides; i++) {
    positions.push(...vec2.rotate(vector, center, i * increment));
    indices.push(index, offset - 1, offset);
    offset++;
  }

  indices.push(index, index + 1, offset - 1);

  return offset;
}

function roundCap(
  positions: number[],
  indices: number[],
  offset: number,
  center: vec2,
  start: vec2,
  end: vec2,
  indexCenter: number,
  indexStart: number,
  indexEnd: number,
  angle: number,
  radius: number,
  zoom: number
) {
  let increment = 2 * Math.acos(1 - GEOMETRY_CURVE_ERROR / (radius * zoom));
  const sides = Math.round(Math.abs(angle) / increment);
  increment = angle / sides;

  positions.push(...vec2.rotate(start, center, increment));
  indices.push(indexCenter, indexStart, offset);
  offset++;

  for (let i = 2; i < sides; i++) {
    positions.push(...vec2.rotate(start, center, i * increment));
    indices.push(indexCenter, offset - 1, offset);
    offset++;
  }

  indices.push(indexCenter, offset - 1, indexEnd);

  return offset;
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

function updateMaxPenDistance(
  i: number,
  p: vec2,
  a: vec2,
  b: vec2,
  maxLeft: PenPointDistance,
  maxRight: PenPointDistance
) {
  const distance = pointLineSquaredDistance(p, [a, b]);
  const axis = vec2.sub(b, a);
  const direction = vec2.sub(p, a);
  const angle = vec2.angle(axis, direction);

  if (angle > 0 && distance > maxRight.distance) {
    maxRight.index = i;
    maxRight.distance = distance;
  } else if (angle < 0 && distance > maxLeft.distance) {
    maxLeft.index = i;
    maxLeft.distance = distance;
  }
}

function getPointFreehandGeometry(
  point: vec3,
  zoom: number,
  size: number
): [Float32Array, number[]] {
  const positions: number[] = [];
  const indices: number[] = [];

  circle(positions, indices, 0, [point[0], point[1]], size * point[2], zoom);

  return [Float32Array.from(positions), indices];
}

function getLineFreehandGeometry(
  a: vec3,
  b: vec3,
  zoom: number,
  size: number
): [Float32Array, number[]] {
  const positions: number[] = [];
  const indices: number[] = [];

  const rA = size * a[2];
  const rB = size * b[2];
  const v = vec2.sub(<any>b, <any>a);
  const n = vec2.perp(v);

  vec2.normalize(n, n);

  const na = vec2.mulS(n, rA);
  const nb = vec2.mulS(n, rB, n);

  const p0 = vec2.add(<any>a, na);
  const p1 = vec2.sub(<any>a, na, na);
  const p2 = vec2.add(<any>b, nb);
  const p3 = vec2.sub(<any>b, nb, nb);

  positions.push(...p0, ...p1, ...p2, ...p3, a[0], a[1], b[0], b[1]);
  indices.push(0, 1, 2, 1, 2, 3);

  let offset = roundCap(positions, indices, 6, <any>a, p0, p1, 4, 0, 1, -Math.PI, rA, zoom);
  roundCap(positions, indices, offset, <any>b, p2, p3, 5, 2, 3, Math.PI, rB, zoom);

  return [Float32Array.from(positions), indices];
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
  const res = 2 * Math.round(vec2.dist(a, b));
  const points = [];

  for (let t = 0; t <= res; ++t) {
    points.push(vec2.lerp(a, b, t / res));
  }

  return pushPoints(positions, indices, offset, points);
}

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

  let offset = 0;
  const points = simplify(inputPoints, 0);

  if (points.length === 1) {
    return getPointFreehandGeometry(points[0], zoom, size);
  }

  if (points.length === 2) {
    return getLineFreehandGeometry(points[0], points[1], zoom, size);
  }

  const debugPoints: vec2[] = [];
  const debugLerpedPoints: Box[] = [];
  const joints: StrokeJoint[] = [];
  const penVertices = 8;

  {
    const point: vec2 = [points[0][0], points[0][1]];
    const next: vec2 = [points[1][0], points[1][1]];
    const width = points[0][2] * size;

    const direction = vec2.dir(point, next);
    const rightNormal = vec2.perp(direction);

    vec2.mulS(rightNormal, width, rightNormal);

    const left = vec2.add(point, rightNormal);
    const right = vec2.sub(point, rightNormal);

    const pen = generateStrokingPen(point, width, penVertices);
    const maxLeft: PenPointDistance = { index: 0, distance: -Infinity };
    const maxRight: PenPointDistance = { index: 0, distance: -Infinity };

    for (let j = 0; j < pen.length; ++j) {
      updateMaxPenDistance(j, pen[j], point, next, maxLeft, maxRight);
    }

    joints.push({
      leftPenIndex: maxLeft.index,
      rightPenIndex: maxRight.index,
      center: point,
      left,
      right,
      prevLeft: left,
      prevRight: right
    });
  }

  for (let i = 1, n = points.length - 1; i < n; ++i) {
    const point: vec2 = [points[i][0], points[i][1]];
    const next: vec2 = [points[i + 1][0], points[i + 1][1]];
    const width = points[i][2] * size;

    const pen = generateStrokingPen(point, width, penVertices);
    const maxLeft: PenPointDistance = { index: 0, distance: -Infinity };
    const maxRight: PenPointDistance = { index: 0, distance: -Infinity };

    for (let j = 0; j < pen.length; ++j) {
      updateMaxPenDistance(j, pen[j], point, next, maxLeft, maxRight);
    }

    const prev = joints[joints.length - 1];

    joints.push({
      leftPenIndex: maxLeft.index,
      rightPenIndex: maxRight.index,
      center: point,
      left: pen[maxLeft.index],
      right: pen[maxRight.index],
      prevLeft: pen[prev.leftPenIndex],
      prevRight: pen[prev.rightPenIndex]
    });
  }

  {
    const point: vec2 = [points[points.length - 1][0], points[points.length - 1][1]];
    const prev: vec2 = [points[points.length - 2][0], points[points.length - 2][1]];
    const width = points[points.length - 1][2] * size;

    const direction = vec2.dir(prev, point);
    const rightNormal = vec2.perp(direction);

    vec2.mulS(rightNormal, width, rightNormal);

    const left = vec2.add(point, rightNormal);
    const right = vec2.sub(point, rightNormal);

    joints.push({
      leftPenIndex: 0,
      rightPenIndex: 0,
      center: point,
      left,
      right,
      prevLeft: left,
      prevRight: right
    });
  }

  const parsedJoints: StrokeParsedJoint[] = [];

  {
    const joint = joints[0];

    positions.push(...joint.left, ...joint.right);
    offset += 2;
  }

  for (let i = 1, n = joints.length - 1; i < n; ++i) {
    const prev = joints[i - 1];
    const joint = joints[i];
    const next = joints[i + 1];

    let prevLeftIndex = 0;
    let prevRightIndex = 0;
    let leftIndex = 0;
    let rightIndex = 0;

    let collapsedLeft = false;
    let collapsedRight = false;

    const isLeftEqual = vec2.equals(joint.prevLeft, joint.left);
    const isRightEqual = vec2.equals(joint.prevRight, joint.right);

    if (isLeftEqual && isRightEqual) {
      positions.push(...joint.left, ...joint.right);

      prevLeftIndex = offset - 2;
      prevRightIndex = offset - 1;
      leftIndex = offset;
      rightIndex = offset + 1;

      collapsedLeft = true;
      collapsedRight = true;
      offset += 2;
    } else {
      let leftIntersection =
        getLineLineIntersection([prev.left, joint.prevLeft], [joint.left, next.prevLeft]) ||
        getLineLineIntersection([prev.left, joint.prevLeft], [joint.right, joint.left]) ||
        getLineLineIntersection([joint.left, next.prevLeft], [joint.prevRight, joint.prevLeft]);

      if (leftIntersection) {
        positions.push(...joint.prevRight, ...leftIntersection, ...joint.right);

        prevLeftIndex = offset - 2;
        prevRightIndex = offset - 1;
        leftIndex = offset + 1;
        rightIndex = offset;

        collapsedLeft = true;
        offset += 3;
      } else {
        let rightIntersection =
          getLineLineIntersection([prev.right, joint.prevRight], [joint.right, next.prevRight]) ||
          getLineLineIntersection([prev.right, joint.prevRight], [joint.right, joint.left]) ||
          getLineLineIntersection([joint.right, next.prevRight], [joint.prevRight, joint.prevLeft]);

        if (rightIntersection) {
          positions.push(...joint.prevLeft, ...joint.left, ...rightIntersection);

          prevLeftIndex = offset - 2;
          prevRightIndex = offset - 1;
          leftIndex = offset;
          rightIndex = offset + 2;

          collapsedRight = true;
          offset += 3;
        } else {
          positions.push(...joint.prevLeft, ...joint.prevRight, ...joint.left, ...joint.right);

          prevLeftIndex = offset - 2;
          prevRightIndex = offset - 1;
          leftIndex = offset;
          rightIndex = offset + 1;

          offset += 4;
        }
      }
    }

    parsedJoints.push({
      prevLeftIndex,
      prevRightIndex,
      leftIndex,
      rightIndex,
      collapsedLeft,
      collapsedRight
    });
  }

  {
    const joint = joints[joints.length - 1];

    positions.push(...joint.left, ...joint.right);

    parsedJoints.push({
      prevLeftIndex: offset - 2,
      prevRightIndex: offset - 1,
      leftIndex: offset,
      rightIndex: offset + 1,
      collapsedLeft: false,
      collapsedRight: false
    });

    offset += 2;
  }

  {
    const joint = joints[0];
    const parsedJoint = parsedJoints[0];

    positions.push(...joint.center);
    offset++;

    offset = roundCap(
      positions,
      indices,
      offset,
      joint.center,
      joint.right,
      joint.left,
      offset - 1,
      parsedJoint.prevRightIndex,
      parsedJoint.prevLeftIndex,
      Math.PI,
      points[0][2] * size,
      zoom
    );
  }

  for (let i = 0, n = parsedJoints.length; i < n; ++i) {
    const joint = parsedJoints[i];
    const next = parsedJoints[i + 1];

    indices.push(
      joint.leftIndex,
      joint.rightIndex,
      joint.prevLeftIndex,
      joint.rightIndex,
      joint.prevLeftIndex,
      joint.prevRightIndex
    );

    if (joint.collapsedLeft && joint.collapsedRight) continue;

    if (joint.collapsedLeft) {
      // indices.push(joint.leftIndex, joint.rightIndex, next.prevRightIndex);

      const center: vec2 = [positions[joint.leftIndex * 2], positions[joint.leftIndex * 2 + 1]];
      const start: vec2 = [positions[joint.rightIndex * 2], positions[joint.rightIndex * 2 + 1]];
      const end: vec2 = [
        positions[next.prevRightIndex * 2],
        positions[next.prevRightIndex * 2 + 1]
      ];

      // debugPoints.push(center, start, end);

      const startDirection = vec2.sub(start, center);
      const endDirection = vec2.sub(end, center);

      let angle = vec2.angle(startDirection, endDirection);

      if (equals(angle, 0)) continue;

      const radius = vec2.len(startDirection);

      // console.log(angle);

      offset = roundCap(
        positions,
        indices,
        offset,
        center,
        start,
        end,
        joint.leftIndex,
        joint.rightIndex,
        next.prevRightIndex,
        angle,
        radius,
        zoom
      );
    } else if (joint.collapsedRight) {
      indices.push(joint.rightIndex, joint.leftIndex, next.prevLeftIndex);
    } else if (next) {
      indices.push(
        joint.rightIndex,
        joint.leftIndex,
        next.prevLeftIndex,
        joint.leftIndex,
        joint.rightIndex,
        next.prevRightIndex
      );
    }
  }

  {
    const joint = joints[joints.length - 1];
    const parsedJoint = parsedJoints[parsedJoints.length - 1];

    positions.push(...joint.center);
    offset++;

    offset = roundCap(
      positions,
      indices,
      offset,
      joint.center,
      joint.left,
      joint.right,
      offset - 1,
      parsedJoint.leftIndex,
      parsedJoint.rightIndex,
      Math.PI,
      points[0][2] * size,
      zoom
    );
  }

  // DEBUG
  if (debugPoints.length) offset = pushPoints(positions, indices, offset, debugPoints);
  if (debugLerpedPoints.length) {
    for (const pair of debugLerpedPoints) {
      offset = pushLerpPoints(positions, indices, offset, pair[0], pair[1]);
    }
  }

  return [Float32Array.from(positions), indices];
}
