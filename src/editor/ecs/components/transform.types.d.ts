interface TransformComponentValue<T> extends Value<T> {
  static: T;
  delta: T;

  apply(): void;
}

interface SimpleTransformComponent {
  readonly position: TransformComponentValue<vec2>;

  readonly boundingBox: Box;

  readonly mat3: mat3;

  translate(amount: vec2, apply?: boolean): void;
  transform(point: vec2): vec2;

  apply(): void;
  asObject(): TransformComponentObject;
}

interface TransformComponent {
  readonly position: TransformComponentValue<vec2>;
  readonly rotation: TransformComponentValue<number>;

  origin: vec2;

  readonly size: vec2;
  readonly center: vec2;
  readonly staticCenter: vec2;

  readonly staticBoundingBox: Box;
  readonly unrotatedBoundingBox: Box;
  readonly rotatedBoundingBox: [vec2, vec2, vec2, vec2];
  readonly boundingBox: Box;

  readonly mat3: mat3;

  translate(amount: vec2, apply?: boolean): void;
  rotate(amount: number, apply?: boolean): void;
  scale(amount: vec2, normalizeRotation?: boolean, apply?: boolean): void;
  transform(point: vec2): vec2;

  apply(): void;
  asObject(): TransformComponentObject;
}

interface ElementTransformComponent extends TransformComponent {
  readonly largeBoundingBox: Box;

  keepCentered(center: vec2, apply?: boolean): void;
}

interface RectTransformComponent extends TransformComponent {
  size: vec2;
  reflection: Value<vec2>;
  scaling: vec2;

  readonly staticSize: vec2;
}

interface VertexTransformComponent {
  readonly position: TransformComponentValue<vec2>;
  readonly left: TransformComponentValue<vec2> | undefined;
  readonly right: TransformComponentValue<vec2> | undefined;

  leftValue: vec2;
  rightValue: vec2;

  readonly boundingBox: Box;

  translate(amount: vec2, apply?: boolean): void;
  translateLeft(amount: vec2, lockMirror?: boolean, apply?: boolean): void;
  translateRight(amount: vec2, lockMirror?: boolean, apply?: boolean): void;
  transform(point: vec2): vec2;

  apply(): void;
}

type GenericTransformComponent =
  | SimpleTransformComponent
  | TransformComponent
  | VertexTransformComponent;

interface TransformComponentObject {
  position?: vec2;
  rotation?: number;
  reflection?: vec2;
}
