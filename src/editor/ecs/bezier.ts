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
  public parent: Entity;

  private m_start: Vertex;
  private m_end: Vertex;

  private m_cache: Cache = new Cache();
  private cached = this.m_cache.cached.bind(this.m_cache);

  constructor({ start, end }: BezierOptions) {
    this.id = nanoid();
    this.m_start = start as Vertex;
    this.m_end = end as Vertex;
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

  private get m_type(): BezierType {
    return this.cached<BezierType>('type', () => {
      let type: BezierType = 'linear';
      if (this.m_start.right || this.m_end.left) {
        if (this.m_start.right && this.m_end.left) type = 'cubic';
        else type = 'quadratic';
      }
      return type;
    });
  }

  get extrema(): vec2[] {
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
    return this.cached<Box>('extrema', () => {
      let min: vec2 = [Infinity, Infinity];
      let max: vec2 = [-Infinity, -Infinity];
      const extrema = this.extrema;
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

  public get visible() {
    return true;
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
    if (lenSquare !== 0) {
      param = dot / lenSquare;
    }

    let vec: vec2;
    if (param < 0) {
      vec = this.start;
    } else if (param > 1) {
      vec = this.end;
    } else {
      vec = vec2.add(this.start, [param * c, param * d]);
    }
    return { t: 0, point: vec, distance: 0 };
  }
  private getBezierClosestTo({
    position,
    iterations
  }: {
    position: vec2;
    iterations: number;
  }): BezierPointDistance {
    const LUT: BezierPointDistance[] = [];
    const size = this.size;
    let min: BezierPointDistance = {
      t: 0,
      point: [0, 0],
      distance: Infinity
    };
    let minID: number = 0;
    const subdivisions = Math.max(Math.floor((size[0] + size[1]) / 50), 10);
    for (let t = 0; t <= subdivisions; t++) {
      const point = this.getPoint(t / subdivisions);
      const distance = vec2.dist(position, point);
      LUT.push({ t: t / subdivisions, distance, point });
      if (distance < min.distance) {
        min = LUT[t];
        minID = t;
      }
    }

    let start = LUT[minID];
    let end: BezierPointDistance;
    if (minID === subdivisions) end = LUT[minID - 1];
    else if (minID === 0) end = LUT[1];
    else end = LUT[minID + 1].distance < LUT[minID - 1].distance ? LUT[minID + 1] : LUT[minID - 1];

    const midT = (start.t + end.t) / 2;
    const midPoint = this.getPoint(midT);
    let mid: BezierPointDistance = {
      t: midT,
      point: midPoint,
      distance: vec2.dist(midPoint, position)
    };

    for (let i = 0; i < iterations; i++) {
      if (start.distance > end.distance) start = end;
      end = mid;
      const t = (start.t * end.distance + end.t * start.distance) / (start.distance + end.distance);
      const point = this.getPoint(t);
      const distance = vec2.dist(point, position);

      mid = { t, point, distance };
    }

    return mid;
  }
  private closestTo(position: vec2, iterations = 2): vec2 {
    return this.call(this.getLinearClosestTo, this.getBezierClosestTo, this.getBezierClosestTo, {
      position,
      iterations
    }).point;
  }

  public distanceTo(position: vec2, iterations = 2): number {
    return vec2.dist(position, this.closestTo(position, iterations));
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
    const data = this.getBezierClosestTo({ position, iterations: 2 });
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
    const data = this.getBezierClosestTo({ position, iterations: 2 });
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
    return this.call(this.getLinearDrawOp, this.getQuadraticDrawOp, this.getCubicDrawOp);
  }

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

  public delete() {}
  public render() {}
  public toJSON() {
    return {} as BezierObject;
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
}

export default Bezier;
