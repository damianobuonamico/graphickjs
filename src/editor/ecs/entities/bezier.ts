import { Cache } from '@utils/cache';
import { doesBoxIntersectBox, getLinesFromBox, isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import Vertex from './vertex';
import { GEOMETRY_MAX_ERROR, GEOMETRY_MAX_INTERSECTION_ERROR } from '@/utils/constants';
import { UntrackedSimpleTransform } from '../components/transform';
import Debugger from '@/utils/debugger';

class Bezier implements BezierEntity {
  readonly id: string;
  readonly type: EntityType = 'bezier';
  readonly selectable = false;
  readonly start: VertexEntity;
  readonly end: VertexEntity;

  private m_parent: ElementEntity;
  transform: UntrackedSimpleTransformComponent;

  private m_cache: Cache = new Cache();

  constructor({ start, end }: BezierOptions) {
    this.id = nanoid();
    this.start = start;
    this.start.registerCache(this.m_cache);
    this.end = end;
    this.end.registerCache(this.m_cache);
    this.transform = new UntrackedSimpleTransform();
  }

  get parent() {
    return this.m_parent;
  }

  set parent(parent: ElementEntity) {
    this.m_parent = parent;
  }

  get p0(): vec2 {
    return this.start.transform.position;
  }

  get p1(): vec2 {
    return this.start.right ? vec2.add(this.p0, this.start.transform.right) : this.p0;
  }

  get p2(): vec2 {
    return this.end.left ? vec2.add(this.p3, this.end.transform.left) : this.p3;
  }

  get p3(): vec2 {
    return this.end.transform.position;
  }

  get bezierType(): BezierType {
    if (this.start.right || this.end.left) return 'cubic';
    return 'linear';
  }

  private onExtremaCacheMiss(): vec2[] {
    const roots = this.getRoots();
    const extrema: vec2[] = [];

    for (const root of roots) {
      extrema.push(this.getPoint(root));
    }

    return extrema;
  }

  get extrema(): vec2[] {
    Debugger.time('extr');
    const extrema = this.m_cache.cached('extrema', this.onExtremaCacheMiss.bind(this));
    Debugger.timeEnd('extr');

    return extrema;
  }

  private onBoundingBoxCacheMiss(): Box {
    const extrema = this.extrema;

    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    for (const point of extrema) {
      min = vec2.min(min, point);
      max = vec2.max(max, point);
    }

    return [min, max];
  }

  get boundingBox(): Box {
    Debugger.time('bbBox');
    const box = this.m_cache.cached('boundingBox', this.onBoundingBoxCacheMiss.bind(this));
    Debugger.timeEnd('bbBox');
    return box;
  }

  get size(): vec2 {
    const box = this.boundingBox;
    return vec2.sub(box[1], box[0]);
  }

  private onClockwiseCacheMiss(): boolean {
    let sum = 0;
    let last = this.getPoint(0);

    for (let i = 1; i <= 100; i++) {
      const point = this.getPoint(i / 100);
      sum += (point[0] - last[0]) * (point[1] + last[1]);
      last = point;
    }

    return sum >= 0;
  }

  get clockwise(): boolean {
    Debugger.time('clock');
    const value = this.m_cache.cached('clockwise', this.onClockwiseCacheMiss.bind(this));
    Debugger.timeEnd('clock');
    return value;
  }

  private call<T>(
    linearFn: (...args: any) => T,
    cubicFn: (...args: any) => T,
    args?: { [key: string]: any }
  ): T {
    if (this.bezierType === 'linear') return linearFn.bind(this)(args);
    else return cubicFn.bind(this)(args);
  }

  recalculate(): void {
    this.m_cache.clear();
  }

  private getLinearPoint({ t = 0 }): vec2 {
    return vec2.add(vec2.mulS(this.p0, 1 - t), vec2.mulS(this.p3, t));
  }
  private getCubicPoint({ t = 0 }): vec2 {
    return vec2.add(
      vec2.add(
        vec2.add(
          vec2.mulS(this.p0, Math.pow(1 - t, 3)),
          vec2.mulS(this.p1, 3 * t * Math.pow(1 - t, 2))
        ),
        vec2.mulS(this.p2, 3 * Math.pow(t, 2) * (1 - t))
      ),
      vec2.mulS(this.p3, Math.pow(t, 3))
    );
  }
  getPoint(t: number): vec2 {
    return this.call(this.getLinearPoint, this.getCubicPoint, {
      t
    });
  }

  private getLinearRoots(): number[] {
    return [0, 1];
  }
  private getCubicRoots(): number[] {
    const a = this.p0;
    const b = this.p1;
    const c = this.p2;
    const d = this.p3;

    const roots: number[] = [0, 1];

    for (let i = 0; i < 2; i++) {
      const sqrtDelta = Math.sqrt(
        Math.pow(b[i], 2) +
          Math.pow(c[i], 2) -
          a[i] * c[i] +
          a[i] * d[i] -
          b[i] * c[i] -
          b[i] * d[i]
      );

      if (sqrtDelta === undefined) continue;

      const num = -a[i] + 2 * b[i] - c[i];
      const den = -a[i] + 3 * b[i] - 3 * c[i] + d[i];
      const t1 = (num + sqrtDelta) / den;

      if (t1 > 0 && t1 < 1) roots.push(t1);
      if (sqrtDelta === 0) continue;

      const t2 = (num - sqrtDelta) / den;

      if (t2 > 0 && t2 < 1) roots.push(t2);
    }

    return roots;
  }
  getRoots(): number[] {
    return this.call(this.getLinearRoots, this.getCubicRoots);
  }

  private getRotatedCubicRoots({ origin, angle }: { origin: vec2; angle: number }): number[] {
    const a = vec2.rotate(this.p0, origin, angle);
    const b = vec2.rotate(this.p1, origin, angle);
    const c = vec2.rotate(this.p2, origin, angle);
    const d = vec2.rotate(this.p3, origin, angle);

    const roots: number[] = [0, 1];

    for (let i = 0; i < 2; i++) {
      const sqrtDelta = Math.sqrt(
        Math.pow(b[i], 2) +
          Math.pow(c[i], 2) -
          a[i] * c[i] +
          a[i] * d[i] -
          b[i] * c[i] -
          b[i] * d[i]
      );

      if (sqrtDelta === undefined) continue;

      const num = -a[i] + 2 * b[i] - c[i];
      const den = -a[i] + 3 * b[i] - 3 * c[i] + d[i];
      const t1 = (num + sqrtDelta) / den;

      if (t1 > 0 && t1 < 1) roots.push(t1);
      if (sqrtDelta === 0) continue;

      const t2 = (num - sqrtDelta) / den;

      if (t2 > 0 && t2 < 1) roots.push(t2);
    }

    return roots;
  }
  getRotatedRoots(origin: vec2, angle: number): number[] {
    return this.call(this.getLinearRoots, this.getRotatedCubicRoots, {
      origin,
      angle
    });
  }

  private onRotatedExtremaCacheMiss(origin: vec2, angle: number) {
    const roots = this.getRotatedRoots(origin, angle);
    const extrema: vec2[] = [];

    for (const root of roots) {
      extrema.push(vec2.rotate(this.getPoint(root), origin, angle));
    }

    return extrema;
  }

  getRotatedExtrema(origin: vec2, angle: number): vec2[] {
    Debugger.time('rExtr');
    const extrema = this.m_cache.cached(
      `extrema-${angle}`,
      this.onRotatedExtremaCacheMiss.bind(this, origin, angle)
    );
    Debugger.timeEnd('rExtr');
    return extrema;
    // });
  }

  private getLinearClosestTo({ position }: { position: vec2 }): BezierPointDistance {
    const [a, b] = vec2.sub(position, this.p0);
    const [c, d] = vec2.sub(this.p3, this.p0);

    const dot = a * c + b * d;
    const lenSquare = c * c + d * d;

    let param = -1;

    if (lenSquare !== 0) param = dot / lenSquare;

    let vec: vec2;
    if (param < 0) vec = this.p0;
    else if (param > 1) vec = this.p3;
    else vec = vec2.add(this.p0, [param * c, param * d]);

    return { t: 0, point: vec, distance: vec2.dist(position, vec) };
  }
  private getCubicClosestTo({
    position,
    iterations
  }: {
    position: vec2;
    iterations: number;
  }): BezierPointDistance {
    const A = this.p0;
    const B = this.p1;
    const C = this.p2;
    const D = this.p3;

    let a = 0;
    let b = 0;
    let c = 0;
    let d = 0;
    let e = 0;
    let f = 0;

    for (let i = 0; i < 2; i++) {
      a +=
        6 * Math.pow(A[i], 2) -
        36 * A[i] * B[i] +
        36 * A[i] * C[i] -
        12 * A[i] * D[i] +
        54 * Math.pow(B[i], 2) -
        108 * B[i] * C[i] +
        36 * B[i] * D[i] +
        54 * Math.pow(C[i], 2) -
        36 * C[i] * D[i] +
        6 * Math.pow(D[i], 2);

      b +=
        -30 * Math.pow(A[i], 2) +
        150 * A[i] * B[i] -
        120 * A[i] * C[i] +
        30 * A[i] * D[i] -
        180 * Math.pow(B[i], 2) +
        270 * B[i] * C[i] -
        60 * B[i] * D[i] -
        90 * Math.pow(C[i], 2) +
        30 * C[i] * D[i];

      c +=
        60 * Math.pow(A[i], 2) -
        240 * A[i] * B[i] +
        144 * A[i] * C[i] -
        24 * A[i] * D[i] +
        216 * Math.pow(B[i], 2) -
        216 * B[i] * C[i] +
        24 * B[i] * D[i] +
        36 * Math.pow(C[i], 2);

      d +=
        -60 * Math.pow(A[i], 2) +
        180 * A[i] * B[i] -
        72 * A[i] * C[i] +
        6 * A[i] * D[i] +
        6 * A[i] * position[i] -
        108 * Math.pow(B[i], 2) +
        54 * B[i] * C[i] -
        18 * B[i] * position[i] +
        18 * C[i] * position[i] -
        6 * D[i] * position[i];

      e +=
        30 * Math.pow(A[i], 2) -
        60 * A[i] * B[i] +
        12 * A[i] * C[i] -
        12 * A[i] * position[i] +
        18 * Math.pow(B[i], 2) +
        24 * B[i] * position[i] -
        12 * C[i] * position[i];

      f +=
        -6 * Math.pow(A[i], 2) + 6 * A[i] * B[i] + 6 * A[i] * position[i] - 6 * B[i] * position[i];
    }

    let t: BezierPointDistance = {
      t: 0,
      point: A,
      distance: vec2.sqrDist(A, position)
    };

    for (let i = 0; i <= iterations; i++) {
      let x = i / iterations;

      for (let i = 0; i < 5; i++) {
        x =
          x -
          (a * Math.pow(x, 5) +
            b * Math.pow(x, 4) +
            c * Math.pow(x, 3) +
            d * Math.pow(x, 2) +
            e * x +
            f) /
            (5 * a * Math.pow(x, 4) +
              4 * b * Math.pow(x, 3) +
              3 * c * Math.pow(x, 2) +
              2 * d * x +
              e);
      }

      if (x < 0 || x > 1) continue;

      const point = this.getCubicPoint({ t: x });
      const dist = vec2.sqrDist(point, position);

      if (dist < t.distance) {
        t.t = x;
        t.point = point;
        t.distance = dist;
      }
    }

    t.distance = Math.sqrt(t.distance);

    return t;
  }
  getClosestTo(position: vec2, iterations: number = 4): vec2 {
    return this.call(this.getLinearClosestTo, this.getCubicClosestTo, {
      position,
      iterations
    }).point;
  }

  getDistanceTo(position: vec2, iterations: number = 8): number {
    return this.call(this.getLinearClosestTo, this.getCubicClosestTo, {
      position,
      iterations
    }).distance;
  }

  private getLinearLineIntersections({ line }: { line: Box }): number[] {
    const m = (line[1][1] - line[0][1]) / (line[1][0] - line[0][0]);

    let roots: number[] = [];

    if (Math.abs(m) === Infinity) {
      roots.push((line[0][0] - this.p0[0]) / (this.p3[0] - this.p0[0]));
    } else {
      roots.push(
        (m * line[0][0] - line[0][1] + this.p0[1] - m * this.p0[0]) /
          (m * (this.p3[0] - this.p0[0]) + this.p0[1] - this.p3[1])
      );
    }

    return roots.filter((root) => 0 <= root && root <= 1);
  }
  private getCubicLineIntersections({ line }: { line: Box }): number[] {
    const left = this.p1;
    const right = this.p2;
    const m = (line[1][1] - line[0][1]) / (line[1][0] - line[0][0]);

    let a: number, b: number, c: number, d: number;
    let roots: number[] = [];

    if (Math.abs(m) === Infinity) {
      a = -this.p0[0] + 3 * left[0] - 3 * right[0] + this.p3[0];
      b = 3 * this.p0[0] - 6 * left[0] + 3 * right[0];
      c = -3 * this.p0[0] + 3 * left[0];
      d = this.p0[0] - line[0][0];
    } else {
      a =
        m * (-this.p0[0] + 3 * left[0] - 3 * right[0] + this.p3[0]) +
        1 * (this.p0[1] - 3 * left[1] + 3 * right[1] - this.p3[1]);
      b =
        m * (3 * this.p0[0] - 6 * left[0] + 3 * right[0]) +
        1 * (-3 * this.p0[1] + 6 * left[1] - 3 * right[1]);
      c = m * (-3 * this.p0[0] + 3 * left[0]) + 1 * (3 * this.p0[1] - 3 * left[1]);
      d = m * (this.p0[0] - line[0][0]) - this.p0[1] + line[0][1];
    }

    // If the cubic bezier is an approximation of a quadratic curve, ignore the third degree term
    if (Math.abs(a) < GEOMETRY_MAX_INTERSECTION_ERROR) {
      const sqrtDelta = Math.sqrt(Math.pow(c, 2) - 4 * b * d);

      if (sqrtDelta === undefined) return [];

      const t1 = (-c + sqrtDelta) / (2 * b);
      const t2 = (-c - sqrtDelta) / (2 * b);

      if (t1 > 0 && t1 < 1) roots.push(t1);
      if (t2 > 0 && t2 < 1 && t2 !== t1) roots.push(t2);

      return roots;
    }

    const p = (3 * a * c - b * b) / (3 * a * a);
    const q = (2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);

    if (Math.abs(p) < GEOMETRY_MAX_ERROR) {
      roots = [Math.cbrt(-q)];
    } else if (Math.abs(q) < GEOMETRY_MAX_ERROR) {
      roots = [0].concat(p < 0 ? [Math.sqrt(-p), -Math.sqrt(-p)] : []);
    } else {
      const D = (q * q) / 4 + (p * p * p) / 27;

      if (Math.abs(D) < GEOMETRY_MAX_ERROR) {
        roots = [(-1.5 * q) / p, (3 * q) / p];
      } else if (D > 0) {
        const u = Math.cbrt(-q / 2 - Math.sqrt(D));

        roots = [u - p / (3 * u)];
      } else {
        const u = 2 * Math.sqrt(-p / 3);
        const t = Math.acos((3 * q) / p / u) / 3;
        const k = (2 * Math.PI) / 3;

        roots = [u * Math.cos(t), u * Math.cos(t - k), u * Math.cos(t - 2 * k)];
      }
    }

    return roots.map((root) => (root -= b / (3 * a))).filter((root) => 0 <= root && root <= 1);
  }
  public getLineIntersections(line: Box): number[] {
    return this.call(this.getLinearLineIntersections, this.getCubicLineIntersections, { line });
  }

  getLineIntersectionPoints(line: Box): vec2[] {
    return this.getLineIntersections(line)
      .map((point) => this.getPoint(point))
      .filter((point) =>
        isPointInBox(
          point,
          [vec2.min(line[0], line[1]), vec2.max(line[0], line[1])],
          GEOMETRY_MAX_INTERSECTION_ERROR
        )
      );
  }

  getBoxIntersectionPoints(box: Box): vec2[] {
    const lines = getLinesFromBox(box);
    const points: vec2[] = [];

    for (const line of lines) {
      points.push(...this.getLineIntersectionPoints(line));
    }

    return points;
  }

  intersectsLine(line: Box): boolean {
    if (
      !doesBoxIntersectBox(
        [vec2.min(line[0], line[1]), vec2.max(line[0], line[1])],
        this.boundingBox
      )
    )
      return false;
    return this.getLineIntersectionPoints(line).length > 0;
  }

  intersectsBox(box: Box): boolean {
    if (!doesBoxIntersectBox(box, this.boundingBox)) return false;
    if (isPointInBox(this.p0, box)) return true;

    return this.getBoxIntersectionPoints(box).length > 0;
  }

  private linearSplit({ position }: { position: vec2 }): VertexEntity {
    return new Vertex({ position: this.getLinearClosestTo({ position }).point });
  }
  private cubicSplit({ position }: { position: vec2 }): VertexEntity {
    const data = this.getCubicClosestTo({ position, iterations: 2 });
    const q0 = vec2.lerp(this.p0, this.p1, data.t);
    const q1 = vec2.lerp(this.p1, this.p2, data.t);
    const q2 = vec2.lerp(this.p2, this.p3, data.t);
    const r0 = vec2.lerp(q0, q1, data.t);
    const r1 = vec2.lerp(q1, q2, data.t);

    this.start.transform.right = vec2.sub(q0, this.p0);
    this.end.transform.left = vec2.sub(q2, this.p3);

    return new Vertex({
      position: data.point,
      left: vec2.sub(r0, data.point),
      right: vec2.sub(r1, data.point)
    });
  }
  split(position: vec2): VertexEntity {
    return this.call(this.linearSplit, this.cubicSplit, {
      position
    });
  }

  destroy(): void {}

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (
      isPointInBox(position, this.boundingBox, threshold) &&
      this.getDistanceTo(position) <= threshold
    )
      return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel: boolean = false): void {}

  private getLinearDrawable(): Drawable {
    return { operations: [{ type: this.bezierType, data: [this.p3] }] };
  }
  private getCubicDrawable(): Drawable {
    return { operations: [{ type: this.bezierType, data: [this.p1, this.p2, this.p3] }] };
  }
  getDrawable(useWebGL: boolean = false): Drawable {
    Debugger.time('bDrwb');
    // return this.cached<Drawable>('getDrawable', () => {
    const value = this.call(this.getLinearDrawable, this.getCubicDrawable);
    Debugger.timeEnd('bDrwb');
    return value;
    // });
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  render(): void {}

  asObject(duplicate: boolean = false): BezierObject {
    return { id: duplicate ? nanoid() : this.id, type: this.type };
  }

  toJSON(): BezierObject {
    return this.asObject(false);
  }
}

export default Bezier;
