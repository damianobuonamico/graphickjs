interface ManipulatorEntity extends Entity {
  transform: UntrackedTransformComponent;
  boundingBox: Box;
  active: boolean;

  set(box: Box | null, angle?: number): void;
}
