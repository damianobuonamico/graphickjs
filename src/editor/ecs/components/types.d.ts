interface StrokeOptions {
  id?: string;
  style?: 'solid' | number[] | null;
  width?: number;
  cap?: CanvasLineCap;
  corner?: CanvasLineJoin;
  miterLimit?: number;
  color?: vec4 | string;
}

interface StrokeComponentObject {
  id?: string;
  style?: 'solid' | number[] | null;
  width?: number;
  cap?: CanvasLineCap;
  corner?: CanvasLineJoin;
  miterLimit?: number;
  color?: vec4;
}

interface StrokeComponent {
  id: string;
  style: 'solid' | number[] | null;
  width: number;
  cap: CanvasLineCap;
  corner: CanvasLineJoin;
  miterLimit: number;
  color: ColorComponent;

  asObject(): StrokeComponentObject;
}

interface FillOptions {
  id?: string;
  style?: 'solid';
  color?: vec4 | string;
}

interface FillComponentObject {
  id?: string;
  style?: 'solid';
  color?: vec4;
}

interface FillComponent {
  id: string;
  style: 'solid';
  color: ColorComponent;

  asObject(): FillComponentObject;
}

type ColorFormat = 'rgb' | 'hsb' | 'hex';

interface ColorComponent {
  readonly hex: string;
  readonly hsb: vec3;
  readonly vec4: vec4;

  alpha: number;

  parse(color: string | vec3 | vec4, format?: ColorFormat): vec3 | vec4 | null;
  tempSet(color: string | vec3 | vec4, format?: ColorFormat): void;
  set(color: string | vec3 | vec4, format?: ColorFormat): void;

  apply(): void;
  clear(): void;

  equals(color: vec4 | string | ColorComponent): boolean;
}

interface TransformComponentValue<T> {
  get(): T;
  getStatic(): T;
  getDelta(): T;

  set(value: T): void;
  add(delta: T): void;
  tempAdd(delta: T): void;

  apply(): void;
  clear(): void;
}

interface BaseTransformComponent {
  position: vec2;
  rotation: number;

  readonly staticPosition: vec2;
  readonly staticRotation: number;

  readonly positionDelta: vec2;
  readonly rotationDelta: number;

  tempTranslate(delta: vec2): void;
  tempRotate(delta: number): void;

  translate(delta: vec2): void;
  rotate(delta: number): void;

  apply(): void;
  clear(): void;
}

interface TransformComponent extends BaseTransformComponent {
  readonly staticCenter: vec2;

  readonly staticBoundingBox: Box;
  readonly unrotatedBoundingBox: Box;
  readonly rotatedBoundingBox: [vec2, vec2, vec2, vec2];
  readonly boundingBox: Box;

  readonly mat3: mat3;

  transform(point: vec2): vec2;
}

interface RectTransformComponent extends TransformComponent {
  size: vec2;

  scaling: vec2;
  reflection: vec2;
  origin: vec2;

  readonly staticSize: vec2;

  tempScale(delta: vec2, correctRotation: boolean): void;

  scale(delta: vec2): void;

  asObject(): TransformComponentObject;
}

interface ElementTransformComponent extends TransformComponent {
  origin: vec2;

  readonly center: vec2;

  readonly largeBoundingBox: Box;

  tempScale(delta: vec2): void;

  scale(delta: vec2): void;

  keepCentered(center: vec2): void;

  asObject(): TransformComponentObject;
}

interface SimpleTransformComponent {
  position: vec2;

  readonly staticPosition: vec2;

  positionDelta: vec2;

  tempPosition: vec2;

  tempTranslate(delta: vec2): void;

  translate(delta: vec2): void;

  readonly mat3: mat3;

  apply(): void;
  clear(): void;
}

interface VertexTransformComponent extends SimpleTransformComponent {
  left: vec2;
  right: vec2;

  readonly staticLeft: vec2;
  readonly staticRight: vec2;

  leftDelta: vec2;
  rightDelta: vec2;

  tempLeft: vec2;
  tempRight: vec2;

  tempTranslateLeft(delta: vec2, lockMirror?: boolean): void;
  tempTranslateRight(delta: vec2, lockMirror?: boolean): void;

  translateLeft(delta: vec2): void;
  translateRight(delta: vec2): void;
}

interface UntrackedBaseTransformComponent {
  position: vec2;
  rotation: number;

  readonly mat3: mat3;

  translate(delta: vec2): void;
  rotate(delta: number): void;
}

interface UntrackedTransformComponent extends UntrackedBaseTransformComponent {
  center: vec2;

  readonly staticBoundingBox: Box;
  readonly unrotatedBoundingBox: Box;
  readonly rotatedBoundingBox: [vec2, vec2, vec2, vec2];
  readonly boundingBox: Box;

  readonly mat3: mat3;

  transform(point: vec2): vec2;
}

interface UntrackedSimpleTransformComponent {
  position: vec2;

  translate(delta: vec2): void;

  readonly mat3: mat3;
}

interface CacheComponent {
  pause: boolean;

  cached<T>(id: string, callback: () => T, genericId?: string): T;
  clear(): void;
}

interface TransformComponentObject {
  position?: vec2;
  rotation?: number;
  reflection?: vec2;
}
