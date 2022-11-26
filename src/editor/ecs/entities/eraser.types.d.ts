interface EraserEntity extends Entity {
  readonly type: 'eraser';
  readonly selectable: false;
  readonly transform: TransformComponent;

  set(position: vec2, pressure: number): void;
}
