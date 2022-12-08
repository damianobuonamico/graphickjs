import { vec2, vec3 } from '@/math';
import { GEOMETRY_CURVE_ERROR } from '@/utils/constants';
import SceneManager from '../scene';

const DEBUG = false;
const DISTANCE_SQR_EPSILON = 0.1;

function generateRoundCap(
  from: vec2,
  to: vec2,
  radius: number,
  vertices: vec2[],
  indices: number[],
  offset: number,
  zoom: number
): number {
  const center = vec2.mid(from, to);
  const directionFrom = vec2.sub(from, center);
  const directionTo = vec2.sub(to, center);
  const angle = vec2.angle(directionFrom, directionTo) || Math.PI;

  let increment = 2 * Math.acos(1 - GEOMETRY_CURVE_ERROR / (radius * zoom));
  let sides = Math.round(Math.abs(angle) / increment);
  increment = angle / sides;

  if (!sides || sides < 1) {
    vertices.push(from);
    vertices.push(to);
    return 2;
  }
  vertices.push(center);
  offset += 1;

  const centerIndex = offset;
  const fromIndex = offset + sides;

  vertices.push(vec2.rotate(from, center, increment));
  indices.push(fromIndex, centerIndex, offset + 1);
  offset += 1;

  for (let i = 2; i < sides; i++) {
    vertices.push(vec2.rotate(from, center, i * increment));
    indices.push(offset + 1, centerIndex, offset);
    offset++;
  }

  vertices.push(from, to);
  indices.push(offset + 2, centerIndex, offset);
  offset += 2;

  return offset - centerIndex + 1;
}

function generateRoundCorner(
  fromIndex: number,
  toIndex: number,
  centerIndex: number,
  center: vec2,
  vertices: vec2[],
  indices: number[],
  offset: number,
  zoom: number
) {
  const from = vertices[fromIndex];
  const to = vertices[toIndex];

  const directionFrom = vec2.sub(from, center);
  const directionTo = vec2.sub(to, center);
  const radius = vec2.len(directionFrom);
  const angle = vec2.angle(directionFrom, directionTo) || Math.PI;

  let increment = 2 * Math.acos(1 - GEOMETRY_CURVE_ERROR / (radius * zoom));
  let sides = Math.round(Math.abs(angle) / increment);
  increment = angle / sides;

  if (!sides || sides < 2) {
    indices.push(fromIndex, toIndex, centerIndex);
    return 0;
  }

  let added = 0;

  vertices.push(vec2.rotate(from, center, increment));
  indices.push(fromIndex, centerIndex, offset + added + 1);
  added += 1;

  for (let i = 2; i < sides; i++) {
    vertices.push(vec2.rotate(from, center, i * increment));
    indices.push(offset + added + 1, centerIndex, offset + added);
    added++;
  }

  vertices.push(from, to);
  indices.push(offset + added + 2, centerIndex, offset + added);
  added += 2;

  return added;
}

function smoothFreehandPoints(points: StrokerPoint[]) {
  const result: StrokerPoint[] = [];
  if (points.length > 2) {
    for (let i = 2, n = points.length; i < n; ++i) {
      const prev2 = points[i - 2];
      const prev1 = points[i - 1];
      const point = points[i];

      const midPoint1 = vec2.mid(prev1.position, prev2.position);
      const midPoint2 = vec2.mid(point.position, prev1.position);

      const segmentDistance = 2;
      const distance = vec2.dist(midPoint1, midPoint2);
      const numberOfSegments = Math.min(128, Math.max(Math.floor(distance / segmentDistance), 2));

      let t = 0;
      const step = 1 / numberOfSegments;

      for (let j = 0; j < numberOfSegments; ++j) {
        const a = Math.pow(1 - t, 2);
        const b = (1 - t) * t;
        const c = t * t;

        result.push({
          position: [
            midPoint1[0] * a + prev1.position[0] * 2 * b + midPoint2[0] * c,
            midPoint1[1] * a + prev1.position[1] * 2 * b + midPoint2[1] * c
          ],
          pressure:
            a * ((prev1.pressure + prev2.pressure) * 0.5) +
            2 * b * prev1.pressure +
            c * ((point.pressure + prev1.pressure) * 0.5)
        });

        t += step;
      }

      result.push({
        position: midPoint2,
        pressure: (point.pressure + prev1.pressure) * 0.5
      });
    }
  }
  return result;
}

function smoothStroke(points: StrokerPoint[], influence: number, iterations: number) {
  if (influence <= 0 || iterations <= 0) {
    return points;
  }

  /* TODO: Currently the weights only control the influence, but is would be much better if they
   * would control the distribution used in smooth, similar to how the ends are handled. */

  /* Perform smoothing. */
  /* If nothing to do, return early */

  const totpoints = points.length;

  if (totpoints <= 2 || iterations <= 0) {
    return points;
  }

  const result = points;

  /* Avoid start and end blobs by averaging them with near points */
  points[points.length - 1].pressure =
    (points[points.length - 1].pressure +
      points[points.length - 2].pressure * 2 +
      points[points.length - 3].pressure * 2) /
    5;
  points[0].pressure = (points[0].pressure + points[1].pressure + points[2].pressure) / 3;

  for (let i = 0; i < totpoints; i++) {
    const pt = points[i];

    // Position
    {
      /* Overview of the algorithm here and in the following smooth functions:
       *  The smooth functions return the new attribute in question for a single point.
       *  The result is stored in r_gps->points[point_index], while the data is read from gps.
       *  To get a correct result, duplicate the stroke point data and read from the copy,
       *  while writing to the real stroke. Not doing that will result in acceptable, but
       *  asymmetric results.
       * This algorithm works as long as all points are being smoothed. If there is
       * points that should not get smoothed, use the old repeat smooth pattern with
       * the parameter "iterations" set to 1 or 2. (2 matches the old algorithm).
       */

      /* If smooth_caps is false, the caps will not be translated by smoothing. */
      // if (!smooth_caps && !is_cyclic && ELEM(point_index, 0, gps->totpoints - 1)) {
      //   copy_v3_v3(&r_gps->points[point_index].x, &pt->x);
      //   return true;
      // }

      /* This function uses a binomial kernel, which is the discrete version of gaussian blur.
       * The weight for a vertex at the relative index point_index is
       * w = nCr(n, j + n/2) / 2^n = (n/1 * (n-1)/2 * ... * (n-j-n/2)/(j+n/2)) / 2^n
       * All weights together sum up to 1
       * This is equivalent to doing multiple iterations of averaging neighbors,
       * where n = iterations * 2 and -n/2 <= j <= n/2
       *
       * Now the problem is that nCr(n, j + n/2) is very hard to compute for n > 500, since even
       * double precision isn't sufficient. A very good robust approximation for n > 20 is
       * nCr(n, j + n/2) / 2^n = sqrt(2/(pi*n)) * exp(-2*j*j/n)
       *
       * There is one more problem left: The old smooth algorithm was doing a more aggressive
       * smooth. To solve that problem, choose a different n/2, which does not match the range and
       * normalize the weights on finish. This may cause some artifacts at low values.
       *
       * keep_shape is a new option to stop the stroke from severely deforming.
       * It uses different partially negative weights.
       * w = 2 * (nCr(n, j + n/2) / 2^n) - (nCr(3*n, j + n) / 2^(3*n))
       *   ~ 2 * sqrt(2/(pi*n)) * exp(-2*j*j/n) - sqrt(2/(pi*3*n)) * exp(-2*j*j/(3*n))
       * All weights still sum up to 1.
       * Note these weights only work because the averaging is done in relative coordinates.
       */
      const keep_shape = true;
      const sco = vec2.create();
      const tmp = vec2.create();
      const n_half = keep_shape
        ? (iterations * iterations) / 8 + iterations
        : (iterations * iterations) / 4 + 2 * iterations + 12;
      let w = keep_shape ? 2.0 : 1.0;
      let w2 = keep_shape
        ? (1.0 / Math.sqrt(3)) * Math.exp((2 * iterations * iterations) / (n_half * 3))
        : 0.0;
      let total_w = 0.0;
      for (let step = iterations; step > 0; step--) {
        let before = i - step;
        let after = i + step;
        let w_before = w - w2;
        let w_after = w - w2;

        if (before < 0) {
          // if (!smooth_caps) {
          //   w_before *= -before / float(point_index);
          // }
          before = 0;
        }
        if (after > totpoints - 1) {
          // if (!smooth_caps) {
          //   w_after *= (after - (gps->totpoints - 1)) / float(gps->totpoints - 1 - point_index);
          // }
          after = totpoints - 1;
        }

        /* Add both these points in relative coordinates to the weighted average sum. */
        vec2.sub(points[before].position, pt.position, tmp);
        vec2.add(sco, vec2.mulS(tmp, w_before), sco);
        vec2.sub(points[after].position, pt.position, tmp);
        vec2.add(sco, vec2.mulS(tmp, w_after), sco);

        total_w += w_before;
        total_w += w_after;

        w *= (n_half + step) / (n_half + 1 - step);
        w2 *= (n_half * 3 + step) / (n_half * 3 + 1 - step);
      }
      total_w += w - w2;
      /* The accumulated weight total_w should be
       * ~sqrt(M_PI * n_half) * exp((iterations * iterations) / n_half) < 100
       * here, but sometimes not quite. */
      vec2.mulS(sco, 1.0 / total_w, sco);
      /* Shift back to global coordinates. */
      vec2.add(sco, pt.position, sco);

      /* Based on influence factor, blend between original and optimal smoothed coordinate. */
      vec2.lerp(pt.position, sco, influence, points[i].position);
    }

    // THICKNESS
    if (SMOOTH_PRESSURE) {
      let pressure = 0;
      const n_half = (iterations * iterations) / 4 + iterations;
      let w = 1.0;
      let total_w = 0.0;
      for (let step = iterations; step > 0; step--) {
        const before = Math.max(i - step, 0);
        const after = Math.min(i + step, totpoints - 1);

        /* Add both these points in relative coordinates to the weighted average sum. */
        pressure += before * (points[before].pressure - pt.pressure);
        pressure += after * (points[after].pressure - pt.pressure);

        total_w += before;
        total_w += after;

        w *= (n_half + step) / (n_half + 1 - step);
      }
      total_w += w;
      /* The accumulated weight total_w should be
       * ~sqrt(M_PI * n_half) * exp((iterations * iterations) / n_half) < 100
       * here, but sometimes not quite. */
      pressure /= total_w;

      /* Based on influence factor, blend between original and optimal smoothed value. */
      result[i].pressure = Math.min(pt.pressure + pressure * influence, 1);
    }
  }

  return result;
}

let ITERATIONS = 1;
let INFLUENCE = 0.5;
let SMOOTH_PRESSURE = true;

const iterationsSlider = document.createElement('input');
iterationsSlider.style.position = 'absolute';
iterationsSlider.style.zIndex = '10000';
iterationsSlider.style.left = '50px';
iterationsSlider.style.top = '40px';
iterationsSlider.type = 'range';
iterationsSlider.min = '0';
iterationsSlider.max = '10';
iterationsSlider.value = `${ITERATIONS}`;
iterationsSlider.step = '1';
document.body.appendChild(iterationsSlider);

const influenceSlider = document.createElement('input');
influenceSlider.style.position = 'absolute';
influenceSlider.style.zIndex = '10000';
influenceSlider.style.left = '50px';
influenceSlider.style.top = '60px';
influenceSlider.type = 'range';
influenceSlider.min = '0';
influenceSlider.max = '2';
influenceSlider.value = `${INFLUENCE}`;
influenceSlider.step = '0.1';
document.body.appendChild(influenceSlider);

const pressureToggle = document.createElement('input');
pressureToggle.style.position = 'absolute';
pressureToggle.style.zIndex = '10000';
pressureToggle.style.left = '50px';
pressureToggle.style.top = '80px';
pressureToggle.type = 'checkbox';
pressureToggle.checked = SMOOTH_PRESSURE;
document.body.appendChild(pressureToggle);

iterationsSlider.addEventListener('input', (event) => {
  ITERATIONS = parseInt(event.target.value);
  SceneManager.viewport.zoom = SceneManager.viewport.zoom + 0.001;
  SceneManager.render();

  setTimeout(() => {
    SceneManager.viewport.zoom = SceneManager.viewport.zoom - 0.001;
    SceneManager.render();
  }, 50);
});

influenceSlider.addEventListener('input', (event) => {
  INFLUENCE = parseFloat(event.target.value);
  SceneManager.viewport.zoom = SceneManager.viewport.zoom + 0.001;
  SceneManager.render();

  setTimeout(() => {
    SceneManager.viewport.zoom = SceneManager.viewport.zoom - 0.001;
    SceneManager.render();
  }, 50);
});

pressureToggle.addEventListener('input', (event) => {
  SMOOTH_PRESSURE = event.target.checked;
  SceneManager.viewport.zoom = SceneManager.viewport.zoom + 0.001;
  SceneManager.render();

  setTimeout(() => {
    SceneManager.viewport.zoom = SceneManager.viewport.zoom - 0.001;
    SceneManager.render();
  }, 50);
});

function smooth(arr: StrokerPoint[]) {
  const windowSize = 5;
  const result: StrokerPoint[] = [...arr];

  for (let i = 0; i < arr.length; i += 1) {
    const leftOffset = i - windowSize;
    const from = leftOffset >= 0 ? leftOffset : 0;
    const to = i + windowSize + 1;

    let count = 0;
    let sum = 0;
    for (let j = from; j < to && j < arr.length; j += 1) {
      sum += arr[j].pressure;
      count += 1;
    }

    result[i].pressure = sum / count;
  }

  return result;
}
function upscaleStroke(points: StrokerPoint[], zoom: number) {
  if (points.length <= 2) return points;

  const result: StrokerPoint[] = [points[0]];

  for (let i = 1, n = points.length - 1; i < n; i++) {
    const prev = points[i - 1];
    const point = points[i];
    const next = points[i + 1];

    const prevPt = prev.position;
    const pointPt = point.position;
    const nextPt = next.position;

    const prevDirection = vec2.sub(pointPt, prevPt);
    const nextDirection = vec2.sub(pointPt, nextPt);

    const prevLength = vec2.len(prevDirection);
    const nextLength = vec2.len(nextDirection);

    const prevMid = vec2.mid(prevPt, pointPt);
    const nextMid = vec2.mid(nextPt, pointPt);

    const M = vec2.mulS(nextDirection, (nextLength + prevLength / 2) / nextLength);
    const N = vec2.mulS(prevDirection, (prevLength + nextLength / 2) / prevLength);

    vec2.add(M, nextPt, M);
    vec2.add(N, prevPt, N);

    const C = vec2.mid(prevMid, M);
    const D = vec2.mid(nextMid, N);

    const E = vec2.mid(C, pointPt);
    const F = vec2.mid(D, pointPt);

    const prevSmooth = vec2.mid(E, prevMid);
    const nextSmooth = vec2.mid(F, nextMid);

    result.push(
      { position: prevSmooth, pressure: (prev.pressure * 2 + point.pressure * 3) / 5 },
      point,
      {
        position: nextSmooth,
        pressure: (next.pressure * 2 + point.pressure * 3) / 5
      }
    );
  }

  result.push(points[points.length - 1]);

  return result;
}

function getPointsGeometry(points: StrokerPoint[]): [Float32Array, number[]] {
  const vertices: number[] = [];
  const indices: number[] = [];

  let offset = 0;

  const width = 0.2;
  for (const point of points) {
    vertices.push(
      point.position[0] - width,
      point.position[1] - width,
      point.position[0] + width,
      point.position[1] - width,
      point.position[0] + width,
      point.position[1] + width,
      point.position[0] - width,
      point.position[1] + width
    );
    indices.push(offset + 0, offset + 1, offset + 2, offset + 2, offset + 3, offset + 0);

    offset += 4;
  }

  return [Float32Array.from(vertices), indices];
}

export function triangulateStroke(
  points: StrokerPoint[],
  thickness: number,
  zoom: number
): [Float32Array, number[]] {
  // points = smoothFreehandPoints(points);
  points = smoothStroke(points, 0.3, 2);
  points = smooth(points);
  points = upscaleStroke(points, zoom);
  // return getPointsGeometry(points);

  const totpoints = points.length;

  /* sanity check */
  if (totpoints < 1) return [Float32Array.from([]), []];

  const strokeRadius = thickness / 2;

  const perimeterRightSide: vec2[] = [];
  const perimeterLeftSide: vec2[] = [];
  const vertices: vec2[] = [];
  const indices: number[] = [];

  const first: StrokerPoint = points[0];
  const last: StrokerPoint = points[totpoints - 1];
  let offset: number = -1;

  const firstRadius: number = strokeRadius * first.pressure;
  const lastRadius: number = strokeRadius * last.pressure;

  let firstNext: StrokerPoint;
  let lastPrev: StrokerPoint;
  if (totpoints > 1) {
    firstNext = points[1];
    lastPrev = points[totpoints - 2];
  } else {
    firstNext = first;
    lastPrev = last;
  }

  const firstPt: vec2 = first.position;
  const lastPt: vec2 = last.position;
  const firstNextPt: vec2 = vec2.clone(firstNext.position);
  const lastPrevPt: vec2 = vec2.clone(lastPrev.position);

  /* Edge-case if single point. */
  if (totpoints == 1) {
    firstNextPt[0] += 0.01;
    lastPrevPt[0] -= 0.01;
  }

  const vecFirst = vec2.sub(firstPt, firstNextPt);

  if (vec2.isZero(vec2.normalize(vecFirst, vecFirst))) {
    vecFirst[0] = 1;
    vecFirst[1] = 0;
  }

  const nvecFirst = vec2.create();

  nvecFirst[0] = -vecFirst[1] * firstRadius;
  nvecFirst[1] = vecFirst[0] * firstRadius;

  /* Generate points for start cap. */
  offset += generateRoundCap(
    vec2.sub(firstPt, nvecFirst),
    vec2.add(firstPt, nvecFirst),
    firstRadius,
    vertices,
    indices,
    offset,
    zoom
  );

  let lastLeftIndex = offset - 1;
  let lastRightIndex = offset;

  /* Generate perimeter points. */
  let currPt = vec2.create(),
    nextPt = vec2.create(),
    prevPt = vec2.create();
  let vecNext = vec2.create(),
    vecPrev = vec2.create();
  let nvecNext = vec2.create(),
    nvecPrev = vec2.create();
  let nvecNextPt = vec2.create(),
    nvecPrevPt = vec2.create();
  let vecTangent = vec2.create();

  let vecMiterLeft = vec2.create(),
    vecMiterRight = vec2.create();
  let miterLeftpt = vec2.create(),
    miterRightPt = vec2.create();

  for (let i = 1; i < totpoints - 1; ++i) {
    const curr: StrokerPoint = points[i];
    const prev: StrokerPoint = points[i - 1];
    const next: StrokerPoint = points[i + 1];
    const radius: number = strokeRadius * curr.pressure;

    vec2.copy(currPt, curr.position);
    vec2.copy(nextPt, next.position);
    vec2.copy(prevPt, prev.position);

    vec2.sub(currPt, prevPt, vecPrev);
    vec2.sub(nextPt, currPt, vecNext);
    const prevLength: number = vec2.len(vecPrev);
    const nextLength: number = vec2.len(vecNext);

    if (vec2.isZero(vec2.normalize(vecPrev, vecPrev))) {
      vecPrev[0] = 1;
      vecPrev[1] = 0;
    }
    if (vec2.isZero(vec2.normalize(vecNext, vecNext))) {
      vecNext[0] = 1;
      vecNext[1] = 0;
    }

    nvecPrev[0] = -vecPrev[1];
    nvecPrev[1] = vecPrev[0];

    nvecNext[0] = -vecNext[1];
    nvecNext[1] = vecNext[0];

    vec2.add(vecPrev, vecNext, vecTangent);
    if (vec2.isZero(vec2.normalize(vecTangent, vecTangent))) {
      vec2.copy(vecTangent, nvecPrev);
    }

    vecMiterLeft[0] = -vecTangent[1];
    vecMiterLeft[1] = vecTangent[0];

    /* calculate miter length */
    let an1: number = vec2.dot(vecMiterLeft, nvecPrev);
    if (an1 === 0) {
      an1 = 1;
    }
    let miterLength = radius / an1;
    if (miterLength <= 0) {
      miterLength = 0.01;
    }

    vec2.normalizeLength(vecMiterLeft, miterLength, vecMiterLeft);

    vec2.copy(vecMiterRight, vecMiterLeft);
    vec2.neg(vecMiterRight, vecMiterRight);

    const angle: number = vec2.dot(vecNext, nvecPrev);
    /* Add two points if angle is close to being straight. */
    if (Math.abs(angle) < 0.0001) {
      vec2.normalizeLength(nvecPrev, radius, nvecPrev);
      vec2.normalizeLength(nvecNext, radius, nvecNext);

      vec2.copy(nvecPrevPt, currPt);
      vec2.add(nvecPrevPt, nvecPrev, nvecPrevPt);

      vec2.copy(nvecNextPt, currPt);
      vec2.neg(nvecNext, nvecNext);
      vec2.add(nvecNextPt, nvecNext, nvecNextPt);

      const normal_prev: vec2 = vec2.clone(nvecPrevPt);
      const normal_next: vec2 = vec2.clone(nvecNextPt);

      perimeterLeftSide.push(normal_prev);
      perimeterRightSide.push(normal_next);

      vertices.push(normal_prev, normal_next);
      offset += 2;

      indices.push(lastLeftIndex, lastRightIndex, offset - 1);
      indices.push(lastRightIndex, offset - 1, offset);

      lastLeftIndex = offset - 1;
      lastRightIndex = offset;
    } else {
      /* bend to the left */
      if (angle < 0) {
        vec2.normalizeLength(nvecPrev, radius, nvecPrev);
        vec2.normalizeLength(nvecNext, radius, nvecNext);

        vec2.copy(nvecPrevPt, currPt);
        vec2.add(nvecPrevPt, nvecPrev, nvecPrevPt);

        vec2.copy(nvecNextPt, currPt);
        vec2.add(nvecNextPt, nvecNext, nvecNextPt);

        const normalPrev: vec2 = vec2.clone(nvecPrevPt);
        const normalNext: vec2 = vec2.clone(nvecNextPt);

        const distance = vec2.sqrDist(normalNext, normalPrev);

        if (distance > DISTANCE_SQR_EPSILON) {
          perimeterLeftSide.push(normalPrev);
          // perimeterLeftSide.push(normalNext);

          vertices.push(normalPrev, normalNext);
          offset += 2;

          indices.push(lastLeftIndex, lastRightIndex, offset - 1);

          lastLeftIndex = offset;

          offset += generateRoundCorner(
            offset - 1,
            offset,
            lastRightIndex,
            currPt,
            vertices,
            indices,
            offset,
            zoom
          );
        } else {
          perimeterLeftSide.push(normalPrev);

          vertices.push(normalPrev);
          offset += 1;

          indices.push(lastLeftIndex, lastRightIndex, offset);

          lastLeftIndex = offset;
        }

        if (miterLength < prevLength && miterLength < nextLength) {
          vec2.copy(miterRightPt, currPt);
          vec2.add(miterRightPt, vecMiterRight, miterRightPt);
        } else {
          vec2.copy(miterRightPt, currPt);
          vec2.neg(nvecNext, nvecNext);
          vec2.add(miterRightPt, nvecNext, miterRightPt);
        }

        const miter_right: vec2 = vec2.clone(miterRightPt);

        perimeterRightSide.push(miter_right);

        vertices.push(miter_right);
        offset += 1;

        indices.push(lastLeftIndex, lastRightIndex, offset);

        lastRightIndex = offset;
      } else {
        vec2.normalizeLength(nvecPrev, -radius, nvecPrev);
        vec2.normalizeLength(nvecNext, -radius, nvecNext);

        vec2.copy(nvecPrevPt, currPt);
        vec2.add(nvecPrevPt, nvecPrev, nvecPrevPt);

        vec2.copy(nvecNextPt, currPt);
        vec2.add(nvecNextPt, nvecNext, nvecNextPt);

        const normalPrev: vec2 = vec2.clone(nvecPrevPt);
        const normalNext: vec2 = vec2.clone(nvecNextPt);

        const distance = vec2.sqrDist(normalNext, normalPrev);

        if (distance > DISTANCE_SQR_EPSILON) {
          perimeterRightSide.push(normalPrev);

          vertices.push(normalPrev, normalNext);
          offset += 2;

          indices.push(lastLeftIndex, lastRightIndex, offset - 1);

          lastRightIndex = offset;

          offset += generateRoundCorner(
            offset - 1,
            offset,
            lastLeftIndex,
            currPt,
            vertices,
            indices,
            offset,
            zoom
          );
        } else {
          perimeterRightSide.push(normalPrev);

          vertices.push(normalPrev);
          offset += 1;

          indices.push(lastLeftIndex, lastRightIndex, offset);

          lastRightIndex = offset;
        }

        // numPerimeterPoints += generate_arc_from_point_to_point(
        //   perimeterRightSide,
        //   normal_prev,
        //   normal_next,
        //   numPerimeterPoints - 2,
        //   numPerimeterPoints - 1,
        //   currPt,
        //   subdivisions,
        //   false
        // );

        if (miterLength < prevLength && miterLength < nextLength) {
          vec2.copy(miterLeftpt, currPt);
          vec2.add(miterLeftpt, vecMiterLeft, miterLeftpt);
        } else {
          vec2.copy(miterLeftpt, currPt);
          vec2.neg(nvecPrev, nvecPrev);
          vec2.add(miterLeftpt, nvecPrev, miterLeftpt);
        }

        const miter_left: vec2 = vec2.clone(miterLeftpt);

        perimeterLeftSide.push(miter_left);

        vertices.push(miter_left);
        offset += 1;

        indices.push(lastLeftIndex, lastRightIndex, offset);

        lastLeftIndex = offset;

        // vertices.push(miter_left);

        // indices.push(lastLeftIndex, lastRightIndex, numPerimeterPoints + 1);

        // lastLeftIndex = ++numPerimeterPoints;
      }
    }
  }

  const vecLast = vec2.sub(lastPrevPt, lastPt);

  if (vec2.isZero(vec2.normalize(vecLast, vecLast))) {
    vecLast[0] = 1;
    vecLast[1] = 0;
  }

  const nvecLast = vec2.create();

  nvecLast[0] = -vecLast[1] * lastRadius;
  nvecLast[1] = vecLast[0] * lastRadius;

  /* Generate points for start cap. */
  offset += generateRoundCap(
    vec2.add(lastPt, nvecLast),
    vec2.sub(lastPt, nvecLast),
    lastRadius,
    vertices,
    indices,
    offset,
    zoom
  );

  indices.push(lastLeftIndex, lastRightIndex, offset - 1);
  indices.push(lastLeftIndex, offset - 1, offset);

  if (DEBUG) {
    const vertices: number[] = [];
    const indices: number[] = [];

    let offset = 0;

    for (const point of [...perimeterLeftSide, ...perimeterRightSide]) {
      const width = strokeRadius * 0.05;

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

  return [Float32Array.from(vertices.flat()), indices];
}

function simplifyRadialDist(points: StrokerPoint[], sqTolerance: number) {
  let prevPoint = points[0];
  let point = prevPoint;

  const newPoints: StrokerPoint[] = [prevPoint];

  for (let i = 1, len = points.length; i < len; i++) {
    point = points[i];

    if (vec2.sqrDist(point.position, prevPoint.position) > sqTolerance) {
      newPoints.push(point);
      prevPoint = point;
    }
  }

  if (prevPoint !== point) newPoints.push(point);

  return newPoints;
}

function getSqSegDist(p: StrokerPoint, p1: StrokerPoint, p2: StrokerPoint) {
  let x = p1.position[0],
    y = p1.position[1],
    dx = p2.position[0] - x,
    dy = p2.position[1] - y;

  if (dx !== 0 || dy !== 0) {
    var t = ((p.position[0] - x) * dx + (p.position[1] - y) * dy) / (dx * dx + dy * dy);

    if (t > 1) {
      x = p2.position[0];
      y = p2.position[1];
    } else if (t > 0) {
      x += dx * t;
      y += dy * t;
    }
  }

  dx = p.position[0] - x;
  dy = p.position[1] - y;

  return dx * dx + dy * dy;
}

function simplifyDPStep(
  points: StrokerPoint[],
  first: number,
  last: number,
  sqTolerance: number,
  simplified: StrokerPoint[]
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

function simplifyDouglasPeucker(points: StrokerPoint[], sqTolerance: number) {
  let last = points.length - 1;

  const simplified = [points[0]];

  simplifyDPStep(points, 0, last, sqTolerance, simplified);
  simplified.push(points[last]);

  return simplified;
}

function simplify(points: StrokerPoint[], tolerance: number) {
  if (points.length <= 2) return points;
  const sqTolerance = tolerance * tolerance;

  return simplifyDouglasPeucker(simplifyRadialDist(points, sqTolerance), sqTolerance);
}
