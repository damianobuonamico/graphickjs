interface ManipulatorEntity extends Entity {
  selectable: false;

  transform: UntrackedTransformComponent;
  boundingBox: Box;
  active: boolean;

  set(box: Box | null, angle?: number): void;
}
