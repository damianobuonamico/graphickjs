interface ManipulatorEntity extends Entity {
  transform: UntrackedTransformComponent;
  active: boolean;

  set(box: Box | null): void;
}
