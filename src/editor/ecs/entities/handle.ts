import { Cache } from '@/editor/ecs/components/cache';
import { isPointInCircle } from '@math';
import { nanoid } from 'nanoid';
import LayerCompositing from '../components/layerCompositing';
import { SimpleTransform } from '../components/transform';

export const isHandle = (b: Entity): b is Handle => {
  return b.type === 'handle';
};

export const isVertexHandle = (b: Entity): b is Handle => {
  return isHandle(b) && b.handleType === 'vertex';
};

class Handle implements HandleEntity {
  readonly id: string;
  readonly type = 'handle';
  readonly selectable = false;
  readonly handleType: HandleType;
  readonly layer: LayerCompositingComponent;

  parent: VertexEntity;
  transform: SimpleTransform;

  constructor({ position, type, parent }: HandleOptions) {
    this.id = nanoid();
    this.handleType = type;
    this.parent = parent;
    this.transform = new SimpleTransform(position);
    this.layer = new LayerCompositing();
  }

  setCache(cache: Cache) {
    this.transform.cache = cache;
  }

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (isPointInCircle(position, this.transform.position.value, threshold)) return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel: boolean = false): void {}

  getDrawable(): Drawable {
    return { operations: [] };
  }

  getOutlineDrawable = this.getDrawable;

  render(): void {}

  asObject(duplicate: boolean = false): HandleObject {
    return { id: duplicate ? nanoid() : this.id, type: this.type };
  }

  toJSON(): EntityObject {
    return this.asObject(false);
  }
}

export default Handle;
