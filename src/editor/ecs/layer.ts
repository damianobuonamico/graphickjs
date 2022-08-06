import { vec2 } from '@math';
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

  public translate() {}

  public delete(entity: Entity) {
    this.remove(entity.id);
  }

  public add(entity: Entity) {
    super.add(entity);
    entity.parent = this;
  }

  public toJSON(duplicate = false) {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      children: this.map((entity) => entity.toJSON(duplicate))
    };
  }
  
  public applyTransform() {}
}

export default Layer;
