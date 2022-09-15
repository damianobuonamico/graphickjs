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

  public get visible() {
    return true;
  }

  public get position() {
    return vec2.create();
  }

  public move() {}

  public moveTo() {}

  public translate() {}

  public applyTransform() {}

  public clearTransform() {}

  public add(entity: Entity, skipRecordAction = false) {
    super.add(entity, skipRecordAction);
    entity.parent = this;
  }

  public delete(entity: Entity) {
    this.remove(entity.id);
  }

  public deleteSelf() {}

  public toJSON(duplicate = false) {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      children: this.map((entity) => entity.toJSON(duplicate))
    };
  }
}

export default Layer;
