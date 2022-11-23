import { nanoid } from 'nanoid';
import LayerCompositing from '../components/layerCompositing';
import { SimpleTransform } from '../components/transform';
import ECS from '../ecs';

class Layer extends ECS implements LayerEntity {
  readonly id: string;
  readonly type = 'layer';
  readonly selectable = false;
  readonly layer: LayerCompositingComponent;
  parent: ArtboardEntity;
  transform: SimpleTransform;

  constructor({ id = nanoid() }: LayerOptions) {
    super();
    this.id = id;
    this.transform = new SimpleTransform();
    this.layer = new LayerCompositing();
  }

  add(entity: Entity) {
    super.add(entity);
    entity.parent = this;
  }

  getDrawable(): Drawable {
    return { operations: [] };
  }

  getOutlineDrawable(): Drawable {
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
