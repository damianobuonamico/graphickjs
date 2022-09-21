import { nanoid } from 'nanoid';
import ECS from '../ecs';

class Layer extends ECS implements LayerEntity {
  readonly id: string;
  readonly type: EntityType = 'layer';

  parent: ArtboardEntity;

  constructor({ id = nanoid() }: LayerOptions) {
    super();
    this.id = id;
  }

  add(entity: Entity, skipRecordAction: boolean = false) {
    super.add(entity, skipRecordAction);
    entity.parent = this;
  }

  delete(entity: Entity, skipRecordAction: boolean = false) {
    this.remove(entity.id, skipRecordAction);
  }

  destroy(): void {}

  getDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  asObject(duplicate: boolean = false): LayerObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      children: this.map((entity) => entity.asObject(duplicate))
    };
  }

  toJSON() {
    return this.asObject(false);
  }
}

export default Layer;
