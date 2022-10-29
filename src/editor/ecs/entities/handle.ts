import { Cache } from '@/editor/ecs/components/cache';
import { isPointInCircle, vec2 } from '@math';
import { nanoid } from 'nanoid';
import { SimpleTransform } from '../components/transform';

class Handle implements HandleEntity {
  readonly id: string;
  readonly type: EntityType = 'handle';
  readonly selectable = false;
  readonly handleType: HandleType;

  parent: VertexEntity;
  transform: SimpleTransform;

  constructor({ position, type, parent }: HandleOptions) {
    this.id = nanoid();
    this.handleType = type;
    this.parent = parent;
    this.transform = new SimpleTransform(position);
  }

  get boundingBox(): Box {
    return [this.transform.position, this.transform.position];
  }

  get staticBoundingBox(): Box {
    return [this.transform.staticPosition, this.transform.staticPosition];
  }

  setCache(cache: Cache) {
    this.transform.cache = cache;
  }

  destroy(): void {}

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (isPointInCircle(position, this.transform.position, threshold)) return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel: boolean = false): void {}

  getDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  render(): void {}

  asObject(duplicate: boolean = false): HandleObject {
    return { id: duplicate ? nanoid() : this.id, type: this.type };
  }

  toJSON(): EntityObject {
    return this.asObject(false);
  }
}

export default Handle;
