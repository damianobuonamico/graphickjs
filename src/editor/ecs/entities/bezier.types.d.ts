interface BezierEntity extends Entity {
  readonly type: 'bezier';
  readonly selectable: false;
  readonly transform: SimpleTransformComponent;

  start: VertexEntity;
  end: VertexEntity;

  p0: vec2;
  p1: vec2;
  p2: vec2;
  p3: vec2;

  bezierType: BezierType;
  extrema: vec2[];
  boundingBox: Box;
  size: vec2;
  clockwise: boolean;

  getPoint(t: number): vec2;
  getRoots(): number[];
  getRotatedRoots(origin: vec2, angle: number): number[];
  getRotatedExtrema(origin: vec2, angle: number): vec2[];
  getClosestTo(position: vec2, iterations?: number): vec2;
  getClosestTTo(position: vec2, iterations?: number): number;
  getDistanceTo(position: vec2, iterations?: number): number;
  getLineIntersections(line: Box): number[];
  getLineIntersectionPoints(line: Box): vec2[];
  getBoxIntersectionPoints(box: Box): vec2[];
  getStrutPoints(t: number): { v1: vec2; A: vec2; v2: vec2; e1: vec2; e2: vec2 };
  getABC(t: number, B: vec2): { A: vec2; B: vec2; C: vec2 };

  intersectsLine(line: Box): boolean;
  intersectsBox(box: Box): boolean;

  split(position: vec2): VertexEntity;
  deriveControlPoints(A: vec2, e1: vec2, e2: vec2, t: number): { p1: vec2; p2: vec2 };
}

type BezierType = 'linear' | 'cubic';

interface BezierOptions {
  start: VertexEntity;
  end: VertexEntity;
}

interface BezierObject extends GenericEntityObject {}

interface BezierPointDistance {
  t: number;
  distance: number;
  point: vec2;
}
