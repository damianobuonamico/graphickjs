import { nanoid } from 'nanoid';
import ECS from './ecs';

class Layer extends ECS implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'layer';
  public parent: Entity;

  constructor({ id = nanoid() }: LayerOptions) {
    super();
    this.id = id;
  }

  public add(entity: Entity) {
    super.add(entity);
    entity.parent = this;
  }

  public toJSON() {
    return {
      id: this.id,
      type: this.type,
      children: this.map((entity) => entity.toJSON())
    };
  }
}

export default Layer;
