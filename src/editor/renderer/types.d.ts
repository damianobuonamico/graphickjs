interface Canvas {
  container: HTMLDivElement;
  DOM: HTMLCanvasElement;

  size: vec2;
  offset: vec2;

  setup(canvas: HTMLCanvasElement): void;
  resize(): void;

  beginFrame(): void;
  endFrame(): void;

  clear(...args: any): void;
  rect({
    pos,
    size,
    centered,
    color,
    transform
  }: {
    pos: vec2;
    size: vec2 | number;
    centered: boolean;
    color: vec4;
    transform?: mat4;
  }): void;
  element(element: Entity): void;

  beginOutline(): void;
  outline(entity: Entity): void;
  endOutline(): void;

  statistics(): void;
}

interface GeometryDrawOp {
  type: 'geometry';
}

interface BezierDrawOp {
  type: BezierType | 'move';
  data: vec2[];
}

interface PathDrawOp {
  type: 'stroke' | 'fill' | 'begin' | 'close';
}

type DrawOp = GeometryDrawOp | BezierDrawOp | PathDrawOp;

interface Drawable {
  operations: DrawOp[];
}

interface CanvasStatsLog {
  entities: number;
  elements: number;
}
