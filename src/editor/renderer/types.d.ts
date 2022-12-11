interface Canvas {
  DOM: HTMLCanvasElement;
  wasmCanvas: HTMLCanvasElement;

  size: vec2;
  offset: vec2;
  primaryColor: string;

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
  debugPoints(id: string, points: vec2[]): void;
  draw(drawable: Drawable): void;

  entity(entity: Entity, options?: { inheritStrokeWidth?: boolean }): void;
  element(element: ElementEntity, options?: { inheritStrokeWidth?: boolean }): void;
  image(image: ImageEntity): void;
  freehand(freehand: FreehandEntity): void;
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

interface Path2DDrawOp {
  type: 'path2D';
  data: [Path2D, boolean, boolean];
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
  | LineDashDrawOp
  | Path2DDrawOp;

interface Drawable {
  operations: DrawOp[];
}

interface DebugState {
  entityBox: boolean;
  segmentBox: boolean;
  vertices: boolean;
  opacity: number;
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

  vertices: number;
  antialiasing: string;

  readonly availableMemory: number;
  readonly hasMemoryStats: boolean;

  begin(): void;
  end(): void;
}
