interface PenEntity extends Entity {
  readonly type: 'pen';
  readonly selectable: false;
  readonly transform: TransformComponent;

  set(options: { p0?: vec2; p1?: vec2; p2?: vec2; p3?: vec2 }): void;
}
