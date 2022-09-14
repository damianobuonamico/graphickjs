import Cache from '@utils/cache';
import { doesBoxIntersectsBox, getLinesFromBox, isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import Handle from './handle';
import Vertex from './vertex';
import { GEOMETRY_MAX_ERROR, GEOMETRY_MAX_INTERSECTION_ERROR } from '@/utils/constants';
import Element from './element';

interface BezierPointDistance {
  t: number;
  distance: number;
  point: vec2;
}

class Bezier implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'bezier';

  public parent: Element;

  private m_start: Vertex;
  private m_end: Vertex;

  private m_cache: Cache = new Cache();
  private cached = this.m_cache.cached.bind(this.m_cache);

  constructor({ start, end }: BezierOptions) {
    this.id = nanoid();
    this.m_start = start as Vertex;
    this.m_end = end as Vertex;
  }

  public get visible() {
    return true;
  }

  public get position() {
    return this.start;
  }

  public get start() {
    return this.m_start.position;
  }

  private get left() {
    return vec2.add(this.start, this.m_start.right!.position);
  }

  private get center() {
    return this.m_start.right ? this.left : this.right;
  }

  private get right() {
    return vec2.add(this.end, this.m_end.left!.position);
  }

  public get end() {
    return this.m_end.position;
  }

  private get m_type() {
    return this.cached<BezierType>('type', () => {
      let type: BezierType = 'linear';

      if (this.m_start.right || this.m_end.left) {
        if (this.m_start.right && this.m_end.left) type = 'cubic';
        else type = 'quadratic';
      }

      return type;
    });
  }

  get extrema() {
    return this.cached<vec2[]>('extrema', () => {
      const roots = this.getRoots();
      const extrema: vec2[] = [];

      for (const root of roots) {
        extrema.push(this.getPoint(root));
      }

      return extrema;
    });
  }

  get boundingBox() {
    return this.cached<Box>('boundingBox', () => {
      const extrema = this.extrema;

      let min: vec2 = [Infinity, Infinity];
      let max: vec2 = [-Infinity, -Infinity];

      for (const point of extrema) {
        min = vec2.min(min, point);
        max = vec2.max(max, point);
      }

      return [min, max];
    });
  }

  get size(): vec2 {
    const box = this.boundingBox;
    return vec2.sub(box[1], box[0]);
  }

  public move(delta: vec2) {
    this.m_start.move(delta);
    this.m_end.move(delta);
  }

  public moveTo() {}

  public translate(delta: vec2) {
    this.m_start.translate(delta);
    this.m_end.translate(delta);
  }

  public applyTransform() {
    this.m_start.applyTransform();
    this.m_end.applyTransform();
  }

  public clearTransform() {
    this.m_start.clearTransform();
    this.m_end.clearTransform();
  }

  public getStart() {
    return this.m_start;
  }

  public getEnd() {
    return this.m_end;
  }

  public recalculate() {
    this.m_cache.clear();
  }

  private call<T>(
    linearFn: (...args: any) => T,
    quadraticFn: (...args: any) => T,
    cubicFn: (...args: any) => T,
    args?: { [key: string]: any }
  ): T {
    const type = this.m_type;

    switch (type) {
      case 'linear':
        return linearFn.bind(this)(args);
      case 'quadratic':
        return quadraticFn.bind(this)(args);
      case 'cubic':
        return cubicFn.bind(this)(args);
    }
  }

  private getLinearPoint({ t = 0 }) {
    return vec2.add(vec2.mul(this.start, 1 - t), vec2.mul(this.end, t));
  }
  private getQuadraticPoint({ t = 0 }) {
    return vec2.add(
      vec2.add(vec2.mul(this.start, Math.pow(1 - t, 2)), vec2.mul(this.center, 2 * t * (1 - t))),
      vec2.mul(this.end, Math.pow(t, 2))
    );
  }
  private getCubicPoint({ t = 0 }) {
    return vec2.add(
      vec2.add(
        vec2.add(
          vec2.mul(this.start, Math.pow(1 - t, 3)),
          vec2.mul(this.left, 3 * t * Math.pow(1 - t, 2))
        ),
        vec2.mul(this.right, 3 * Math.pow(t, 2) * (1 - t))
      ),
      vec2.mul(this.end, Math.pow(t, 3))
    );
  }
  public getPoint(t: number) {
    return this.call(this.getLinearPoint, this.getQuadraticPoint, this.getCubicPoint, {
      t
    });
  }

  private getLinearRoots(): number[] {
    return [0, 1];
  }
  private getQuadraticRoots(): number[] {
    const a = this.start;
    const b = this.center;
    const c = this.end;

    const roots: number[] = [0, 1];

    for (let i = 0; i < 2; i++) {
      const t = (a[i] - b[i]) / (a[i] - 2 * b[i] + c[i]);
      if (t > 0 && t < 1) roots.push(t);
    }

    return roots;
  }
  private getCubicRoots(): number[] {
    const a = this.start;
    const b = this.left;
    const c = this.right;
    const d = this.end;

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
  private getRoots(): number[] {
    return this.call(this.getLinearRoots, this.getQuadraticRoots, this.getCubicRoots);
  }

  private getLinearClosestTo({ position }: { position: vec2 }): BezierPointDistance {
    const [a, b] = vec2.sub(position, this.start) as number[];
    const [c, d] = vec2.sub(this.end, this.start) as number[];

    const dot = a * c + b * d;
    const lenSquare = c * c + d * d;

    let param = -1;

    if (lenSquare !== 0) param = dot / lenSquare;

    let vec: vec2;
    if (param < 0) vec = this.start;
    else if (param > 1) vec = this.end;
    else vec = vec2.add(this.start, [param * c, param * d]);

    return { t: 0, point: vec, distance: vec2.dist(position, vec) };
  }
  private getQuadraticClosestTo({
    position,
    iterations
  }: {
    position: vec2;
    iterations: number;
  }): BezierPointDistance {
    const A = this.start;
    const B = this.center;
    const C = this.end;

    let a = 0;
    let b = 0;
    let c = 0;
    let d = 0;

    for (let i = 0; i < 2; i++) {
      a +=
        4 * Math.pow(A[i], 2) -
        16 * A[i] * B[i] +
        8 * A[i] * C[i] +
        16 * Math.pow(B[i], 2) -
        16 * B[i] * C[i] +
        4 * Math.pow(C[i], 2);

      b +=
        -12 * Math.pow(A[i], 2) +
        36 * A[i] * B[i] -
        12 * A[i] * C[i] -
        24 * Math.pow(B[i], 2) +
        12 * B[i] * C[i];

      c +=
        12 * Math.pow(A[i], 2) -
        24 * A[i] * B[i] +
        4 * A[i] * C[i] -
        4 * A[i] * position[i] +
        8 * Math.pow(B[i], 2) +
        8 * B[i] * position[i] -
        4 * C[i] * position[i];

      d +=
        -4 * Math.pow(A[i], 2) + 4 * A[i] * B[i] + 4 * A[i] * position[i] - 4 * B[i] * position[i];
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
          (a * Math.pow(x, 3) + b * Math.pow(x, 2) + c * x + d) /
            (3 * a * Math.pow(x, 2) + 2 * b * x + c);
      }

      if (x < 0 || x > 1) continue;

      const point = this.getQuadraticPoint({ t: x });
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
  private getCubicClosestTo({
    position,
    iterations
  }: {
    position: vec2;
    iterations: number;
  }): BezierPointDistance {
    const A = this.start;
    const B = this.left;
    const C = this.right;
    const D = this.end;

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
  private closestTo(position: vec2, iterations = 4): vec2 {
    return this.call(this.getLinearClosestTo, this.getQuadraticClosestTo, this.getCubicClosestTo, {
      position,
      iterations
    }).point;
  }

  public distanceTo(position: vec2, iterations = 8): number {
    return this.call(this.getLinearClosestTo, this.getQuadraticClosestTo, this.getCubicClosestTo, {
      position,
      iterations
    }).distance;
  }

  private getLinearLineIntersections({ line }: { line: Box }): number[] {
    const m = (line[1][1] - line[0][1]) / (line[1][0] - line[0][0]);

    let roots: number[] = [];

    if (Math.abs(m) === Infinity) {
      roots.push((line[0][0] - this.start[0]) / (this.end[0] - this.start[0]));
    } else {
      roots.push(
        (m * line[0][0] - line[0][1] + this.start[1] - m * this.start[0]) /
          (m * (this.end[0] - this.start[0]) + this.start[1] - this.end[1])
      );
    }

    return roots.filter((root) => 0 <= root && root <= 1);
  }
  private getQuadraticLineIntersections({ line }: { line: Box }): number[] {
    const m = (line[1][1] - line[0][1]) / (line[1][0] - line[0][0]);
    const bez = this.center;

    let a: number, b: number, c: number;
    let roots: number[] = [];

    if (Math.abs(m) === Infinity) {
      a = this.start[0] - 2 * bez[0] + this.end[0];
      b = -2 * this.start[0] + 2 * bez[0];
      c = this.start[0] - line[0][0];
    } else {
      a =
        m * (this.start[0] - 2 * bez[0] + this.end[0]) +
        1 * (-this.start[1] + 2 * bez[1] - this.end[1]);
      b = 2 * (m * (-this.start[0] + bez[0]) + this.start[1] - bez[1]);
      c = m * (this.start[0] - line[0][0]) - this.start[1] + line[0][1];
    }

    const sqrtDelta = Math.sqrt(Math.pow(b, 2) - 4 * a * c);

    if (sqrtDelta === undefined) return [];

    const t1 = (-b + sqrtDelta) / (2 * a);

    if (t1 > 0 && t1 < 1) roots.push(t1);

    const t2 = (-b - sqrtDelta) / (2 * a);

    if (t2 > 0 && t2 < 1 && t2 !== t1) roots.push(t2);

    return roots;
  }
  private getCubicLineIntersections({ line }: { line: Box }): number[] {
    const left = this.left;
    const right = this.right;
    const m = (line[1][1] - line[0][1]) / (line[1][0] - line[0][0]);

    let a: number, b: number, c: number, d: number;
    let roots: number[] = [];

    if (Math.abs(m) === Infinity) {
      a = -this.start[0] + 3 * left[0] - 3 * right[0] + this.end[0];
      b = 3 * this.start[0] - 6 * left[0] + 3 * right[0];
      c = -3 * this.start[0] + 3 * left[0];
      d = this.start[0] - line[0][0];
    } else {
      a =
        m * (-this.start[0] + 3 * left[0] - 3 * right[0] + this.end[0]) +
        1 * (this.start[1] - 3 * left[1] + 3 * right[1] - this.end[1]);
      b =
        m * (3 * this.start[0] - 6 * left[0] + 3 * right[0]) +
        1 * (-3 * this.start[1] + 6 * left[1] - 3 * right[1]);
      c = m * (-3 * this.start[0] + 3 * left[0]) + 1 * (3 * this.start[1] - 3 * left[1]);
      d = m * (this.start[0] - line[0][0]) - this.start[1] + line[0][1];
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
  public getLineIntersections(line: Box) {
    return this.call(
      this.getLinearLineIntersections,
      this.getQuadraticLineIntersections,
      this.getCubicLineIntersections,
      { line }
    );
  }

  public getLineIntersectionPoints(line: Box) {
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

  public intersectsLine(line: Box): boolean {
    if (!doesBoxIntersectsBox(line, this.boundingBox)) return false;
    return this.getLineIntersections(line).length > 0;
  }

  private getBoxIntersections(box: Box) {
    const lines = getLinesFromBox(box);
    const points: number[] = [];

    for (const line of lines) {
      points.push(...this.getLineIntersections(line));
    }

    return points;
  }

  public getBoxIntersectionPoints(box: Box) {
    const lines = getLinesFromBox(box);
    const points: vec2[] = [];

    for (const line of lines) {
      points.push(...this.getLineIntersectionPoints(line));
    }

    return points;
  }

  public intersectsBox(box: Box) {
    if (!doesBoxIntersectsBox(box, this.boundingBox)) return false;
    if (isPointInBox(this.start, box)) return true;

    return this.getBoxIntersectionPoints(box).length > 0;
  }

  private linearSplit({ position }: { position: vec2 }): Vertex {
    return new Vertex({ position: this.getLinearClosestTo({ position }).point });
  }
  private quadraticSplit({ position }: { position: vec2 }): Vertex {
    const data = this.getQuadraticClosestTo({ position, iterations: 2 });
    const p1 = this.center;
    const q0 = vec2.lerp(this.start, p1, data.t);
    const q1 = vec2.lerp(p1, this.end, data.t);
    const p = vec2.lerp(q0, q1, data.t);

    if (this.m_start.right) {
      const r0 = vec2.add(vec2.mul(this.start, 1 / 3), vec2.mul(q0, 2.00001 / 3));
      const r1 = vec2.add(vec2.mul(q0, 2 / 3), vec2.mul(p, 1 / 3));

      this.m_start.setRight(vec2.sub(r0, this.start));

      return new Vertex({
        position: data.point,
        left: vec2.sub(r1, data.point),
        right: vec2.sub(q1, data.point)
      });
    } else {
      const r0 = vec2.add(vec2.mul(p, 1 / 3), vec2.mul(q1, 2.00001 / 3));
      const r1 = vec2.add(vec2.mul(q1, 2 / 3), vec2.mul(this.end, 1 / 3));

      this.m_start.setLeft(vec2.sub(r1, this.end));

      return new Vertex({
        position: data.point,
        left: vec2.sub(q0, data.point),
        right: vec2.sub(r0, data.point)
      });
    }
  }
  private cubicSplit({ position }: { position: vec2 }): Vertex {
    const data = this.getCubicClosestTo({ position, iterations: 2 });
    const q0 = vec2.lerp(this.start, this.left, data.t);
    const q1 = vec2.lerp(this.left, this.right, data.t);
    const q2 = vec2.lerp(this.right, this.end, data.t);
    const r0 = vec2.lerp(q0, q1, data.t);
    const r1 = vec2.lerp(q1, q2, data.t);

    this.m_start.setRight(vec2.sub(q0, this.start));
    this.m_end.setLeft(vec2.sub(q2, this.end));

    return new Vertex({
      position: data.point,
      left: vec2.sub(r0, data.point),
      right: vec2.sub(r1, data.point)
    });
  }
  public split(position: vec2) {
    return this.call(this.linearSplit, this.quadraticSplit, this.cubicSplit, {
      position
    });
  }

  public getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    if (
      isPointInBox(position, this.boundingBox, threshold) &&
      this.distanceTo(position) <= threshold
    )
      return this;
    return undefined;
  }

  public getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {}

  public delete() {}

  public render() {}

  private getLinearDrawOp() {
    return { type: this.m_type, data: [this.end] };
  }
  private getQuadraticDrawOp() {
    return { type: this.m_type, data: [this.center, this.end] };
  }
  private getCubicDrawOp() {
    return { type: this.m_type, data: [this.left, this.right, this.end] };
  }
  public getDrawOp() {
    return this.cached('getDrawOp', () => {
      return this.call(this.getLinearDrawOp, this.getQuadraticDrawOp, this.getCubicDrawOp);
    });
  }

  public toJSON() {
    return {} as BezierObject;
  }
}

export default Bezier;
