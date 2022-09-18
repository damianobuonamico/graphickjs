import Element from './ecs/element';
import ImageEntity from './ecs/image';

class HoverState {
  public entity: Entity | undefined = undefined;

  public get element(): Element | ImageEntity | undefined {
    let entity = this.entity;

    if (!entity) return undefined;
    while (entity.type !== 'element' && entity.type !== 'image' && entity.parent) {
      entity = entity.parent;
    }

    if (entity.type === 'element' || entity.type === 'image')
      return entity as Element | ImageEntity;

    return undefined;
  }
}

export default HoverState;
