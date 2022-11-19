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
    stats,
    debugging
  }: {
    color?: string;
    zoom?: number;
    position?: vec2;
    stats?: RendererStats;
    debugging?: boolean;
  }): void;
  endFrame({
    stats,
    debugging,
    debug
  }: {
    stats?: RendererStats;
    debugging?: boolean;
    debug?: DebugState;
  }): void;

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

interface RoundedRectDrawOp {
  type: 'roundedRect';
  data: [vec2, vec2, number];
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

interface LineDashDrawOp {
  type: 'lineDash';
  data: [number[] | undefined];
}

type DrawOp =
  | GeometryDrawOp
  | BezierDrawOp
  | PathDrawOp
  | ShapeDrawOp
  | CenteredShapeDrawOp
  | ColorDrawOp
  | ParameterDrawOp
  | RoundedRectDrawOp
  | LineDashDrawOp;

interface Drawable {
  operations: DrawOp[];
}

interface DebugState {
  entityBox: boolean;
  segmentBox: boolean;
  vertices: boolean;
}

interface RendererStats {
  ms: number[];
  fps: number[];
  memory: number[];

  avgMs: number;
  avgFps: number;
  avgMemory: number;

  minMs: number;
  maxMs: number;
  minFps: number;
  maxFps: number;
  minMemory: number;
  maxMemory: number;

  readonly availableMemory: number;
  readonly hasMemoryStats: boolean;

  begin(): void;
  end(): void;
}
