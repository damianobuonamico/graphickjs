interface BezierEntity extends Entity {
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

  recalculate(): void;

  getPoint(t: number): vec2;
  getRoots(): number[];
  getRotatedRoots(origin: vec2, angle: number): number[];
  getRotatedExtrema(origin: vec2, angle: number): vec2[];
  getClosestTo(position: vec2, iterations: number): vec2;
  getDistanceTo(position: vec2, iterations: number): number;
  getLineIntersections(line: Box): number[];
  getLineIntersectionPoints(line: Box): vec2[];
  getBoxIntersectionPoints(box: Box): vec2[];

  intersectsLine(line: Box): boolean;
  intersectsBox(box: Box): boolean;

  split(position: vec2): VertexEntity;
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
