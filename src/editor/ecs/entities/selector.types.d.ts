interface SelectorEntity extends Entity {
  readonly type: 'selector';
  readonly selectable: false;
  readonly transform: TransformComponent;

  set(size: vec2, position?: vec2): void;
}
