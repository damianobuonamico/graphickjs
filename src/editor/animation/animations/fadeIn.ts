export class FadeInAnimation implements AnimationInterface {
  readonly name: 'fadeIn';

  get id(): string {
    return 'opacity';
  }

  animate(entity: Entity, t: number, duration: number) {
    entity.layer.opacity.animateTo(t / duration);
  }

  reset(entity: Entity) {
    entity.layer.opacity.animateTo(null);
  }

  ready(entity: Entity) {
    entity.layer.opacity.animateTo(0);
  }
}
