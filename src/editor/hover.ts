import { isElement } from './ecs/entities/element';
import { isFreehand } from './ecs/entities/freehand';
import { isImage } from './ecs/entities/image';

class HoverState {
  public entity: Entity | undefined = undefined;

  public get element(): ElementEntity | ImageEntity | FreehandEntity | undefined {
    let entity = this.entity;
    if (!entity) return undefined;

    while (!isElement(entity) && !isImage(entity) && !isFreehand(entity) && entity.parent) {
      entity = entity.parent;
    }

    if (isElement(entity) || isImage(entity) || isFreehand(entity)) return entity;

    return undefined;
  }
}

export default HoverState;
