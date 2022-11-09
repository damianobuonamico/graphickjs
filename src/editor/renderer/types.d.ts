interface Canvas {
  DOM: HTMLCanvasElement;

  size: vec2;
  offset: vec2;

  setup(canvas: HTMLCanvasElement): void;
  resize(): void;

  beginFrame({
    color,
    zoom,
    position,
    stats
  }: {
    color?: string;
    zoom?: number;
    position?: vec2;
    stats?: Stats;
  }): void;
  endFrame({ stats, debugging }: { stats?: Stats; debugging?: boolean }): void;

  debugRect({
    position,
    size,
    centered,
    color
  }: {
    position: vec2;
    size?: vec2 | number;
    centered?: boolean;
    color?: string;
  }): void;
  debugCircle({
    position,
    radius,
    color
  }: {
    position: vec2;
    radius?: number;
    color?: string;
  }): void;
  draw(drawable: Drawable): void;

  entity(entity: Entity, options?: { inheritStrokeWidth?: boolean }): void;
  element(element: Entity, options?: { inheritStrokeWidth?: boolean }): void;
  image(image: Entity): void;
  rectangle({
    position,
    size,
    centered,
    stroke,
    fill
  }: {
    position: vec2;
    size: vec2 | number;
    centered?: boolean;
    stroke?: string;
    fill?: string;
  }): void;

  beginOutline(): void;
  outline(entity: Entity, skipVertices?: boolean): void;
  endOutline(): void;
}

interface GeometryDrawOp {
  type: 'geometry';
  data?: undefined;
}

interface BezierDrawOp {
  type: 'lineTo' | 'cubicTo' | 'moveTo';
  data: vec2[];
}

interface PathDrawOp {
  type: 'stroke' | 'fill' | 'beginPath' | 'closePath';
  data?: undefined;
}

interface ShapeDrawOp {
  type: 'rect';
  data: [vec2, vec2];
}

interface CenteredShapeDrawOp {
  type: 'circle' | 'square';
  data: [vec2, number];
}

interface ColorDrawOp {
  type: 'strokeColor' | 'fillColor';
  data: [string];
}

interface ParameterDrawOp {
  type: 'strokeWidth';
  data: [number];
}

type DrawOp =
  | GeometryDrawOp
  | BezierDrawOp
  | PathDrawOp
  | ShapeDrawOp
  | CenteredShapeDrawOp
  | ColorDrawOp
  | ParameterDrawOp;

interface Drawable {
  operations: DrawOp[];
}

interface DebugState {
  box: boolean;
}

interface Stats {
  begin(): void;
  end(): void;
}
