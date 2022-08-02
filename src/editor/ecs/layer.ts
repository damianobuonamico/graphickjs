import { nanoid } from 'nanoid';
import ECS from './ecs';

class Layer extends ECS implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'layer';
  public parent: Entity;

  constructor() {
    super();
    this.id = nanoid();
  }

  public add(entity: Entity) {
    super.add(entity);
    entity.parent = this;
  }
}

export default Layer;
