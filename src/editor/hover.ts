import Element from './ecs/element';

class HoverState {
  public entity: Entity | undefined = undefined;

  public get element(): Element | undefined {
    let entity = this.entity;
    if (!entity) return undefined;
    while (entity.type !== 'element' && entity.parent) {
      entity = entity.parent;
    }
    if (entity.type === 'element') return entity as Element;
    return undefined;
  }
}

export default HoverState;
