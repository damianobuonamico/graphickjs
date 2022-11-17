interface ManipulatorEntity extends Entity {
  readonly type: 'manipulator';
  readonly selectable: false;
  readonly transform: TransformComponent;

  active: boolean;

  set(box: Box | null, angle?: number): void;
}
