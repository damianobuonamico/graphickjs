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
  tempGet(): T;

  set(value: T): void;
  add(delta: T): void;
  translate(delta: T): void;

  apply(): void;
  clear(): void;
}

interface SimpleTransformComponent {
  position: vec2;
  staticPosition: vec2;
  tempPosition: vec2;
  translate(delta: vec2): void;
  tempTranslate(delta: vec2): void;

  mat3: mat3;

  clear(): void;
  apply(): void;
}

interface UntrackedSimpleTransformComponent {
  position: vec2;
  translate(delta: vec2): void;

  mat3: mat3;
}

interface TransformComponent {
  origin: vec2;

  position: vec2;
  staticPosition: vec2;
  tempPosition: vec2;
  translate(delta: vec2): void;
  tempTranslate(delta: vec2): void;

  rotation: number;
  staticRotation: number;
  tempRotation: number;
  rotate(delta: number): void;
  tempRotate(delta: number): void;

  scale(magnitude: vec2): void;
  tempScale(magnitude: vec2): void;

  mat3: mat3;

  clear(): void;
  apply(): void;
}

interface UntrackedTransformComponent {
  origin: vec2;

  position: vec2;
  translate(delta: vec2): void;

  rotation: number;
  rotate(delta: number): void;

  mat3: mat3;
}

interface VertexTransformComponent {
  position: vec2;
  staticPosition: vec2;
  tempPosition: vec2;
  translate(delta: vec2): void;
  tempTranslate(delta: vec2): void;

  left: vec2;
  staticLeft: vec2;
  tempLeft: vec2;
  translateLeft(delta: vec2): void;
  tempTranslateLeft(delta: vec2, lockMirror?: boolean): void;

  right: vec2;
  staticRight: vec2;
  tempRight: vec2;
  translateRight(delta: vec2): void;
  tempTranslateRight(delta: vec2, lockMirror?: boolean): void;

  clear(): void;
  apply(): void;
}
