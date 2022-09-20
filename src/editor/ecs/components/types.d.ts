interface StrokeOptions {
  id?: string;
  style?: 'solid' | number[] | null;
  width?: number;
  cap?: CanvasLineCap;
  corner?: CanvasLineJoin;
  miterLimit?: number;
  color?: vec4 | string;
}

interface Stroke {
  id: string;
  style: 'solid' | number[] | null;
  width: number;
  cap: CanvasLineCap;
  corner: CanvasLineJoin;
  miterLimit: number;
  color: vec4;
}

interface FillOptions {
  id?: string;
  style?: 'solid';
  color?: vec4 | string;
}

interface Fill {
  id: string;
  style: 'solid';
  color: vec4;
}

interface TransformComponentValue<T> {
  get(): T;
  staticGet(): T;

  set(value: T): void;
  add(delta: T): void;
  translate(delta: T): void;

  apply(): void;
  clear(): void;
}

interface SimpleTransformComponent {
  position: vec2;
  staticPosition: vec2;
  translation: vec2;
  move(delta: vec2): void;
  translate(delta: vec2): void;

  mat3: mat3;

  clear(): void;
  apply(): void;
}

interface TransformComponent {
  origin: vec2;

  position: vec2;
  staticPosition: vec2;
  move(delta: vec2): void;
  translate(delta: vec2): void;

  rotation: number;

  scale: vec2;

  mat3: mat3;

  clear(): void;
  apply(): void;
}
