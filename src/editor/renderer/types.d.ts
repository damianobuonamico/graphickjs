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
  draw(drawable: Drawable): void;

  beginOutline(): void;
  outline(entity: Entity): void;
  endOutline(): void;

  debugging(): void;
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

interface ShapeDrawOp {
  type: 'circle' | 'rect';
  data: [vec2, number];
}

interface ColorDrawOp {
  type: 'strokecolor' | 'fillcolor';
  data: vec4;
}

type DrawOp = GeometryDrawOp | BezierDrawOp | PathDrawOp | ShapeDrawOp | ColorDrawOp;

interface Drawable {
  operations: DrawOp[];
}

interface DebugState {
  box: boolean;
}
