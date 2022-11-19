import { Cache, VertexCache } from '@/editor/ecs/components/cache';
import { ChangeCommand } from '@/editor/history/command';
import CommandHistory from '@/editor/history/history';
import { EntityValue } from '@/editor/history/value';
import { isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import { VertexTransform } from '../components/transform';
import Handle from './handle';

export const isVertex = (b: Entity): b is Vertex => {
  return b.type === 'vertex';
};

class Vertex implements VertexEntity {
  readonly id: string;
  readonly type = 'vertex';
  readonly selectable = false;
  readonly transform: VertexTransform;

  private m_parent: EntityValue<ElementEntity> = new EntityValue();

  private m_position: HandleEntity;
  private m_left?: HandleEntity;
  private m_right?: HandleEntity;

  private m_cache: VertexCache = new VertexCache();
  private m_cacheDisabled: boolean;

  constructor({ id = nanoid(), position, left, right, disableCache = false }: VertexOptions) {
    this.id = id;
    this.m_position = new Handle({ position, type: 'vertex', parent: this });
    this.m_cacheDisabled = disableCache;
    if (!disableCache) this.m_position.setCache(this.m_cache);

    if (left) this.left = new Handle({ position: left, type: 'bezier', parent: this });
    if (right) this.right = new Handle({ position: right, type: 'bezier', parent: this });

    this.transform = new VertexTransform(this);
  }

  get parent() {
    return this.m_parent.value!;
  }

  set parent(parent: ElementEntity) {
    this.m_parent.value = parent;
    this.m_cache.parentCache = parent.cache;
  }

  get position(): HandleEntity {
    return this.m_position;
  }

  get left(): HandleEntity | undefined {
    return this.m_left;
  }

  set left(handle: HandleEntity | undefined) {
    const backup = this.m_left;

    if (!this.m_cacheDisabled && handle) handle.setCache(this.m_cache);

    CommandHistory.add(
      new ChangeCommand(
        () => {
          this.m_left = handle;
          this.m_cache.pause = true;
        },
        () => {
          this.m_left = backup;
          this.m_cache.pause = true;
        }
      )
    );

    this.m_left = handle;
  }

  get right(): HandleEntity | undefined {
    return this.m_right;
  }

  set right(handle: HandleEntity | undefined) {
    const backup = this.m_right;

    if (!this.m_cacheDisabled) handle?.setCache(this.m_cache as any as Cache);

    CommandHistory.add(
      new ChangeCommand(
        () => {
          this.m_right = handle;
          this.m_cache.pause = true;
        },
        () => {
          this.m_right = backup;
          this.m_cache.pause = true;
        }
      )
    );
  }

  registerCache(cache: Cache): void {
    this.m_cache.register(cache);
  }

  pauseCache(): void {
    this.m_cache.pause = true;
  }

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (this.m_position.getEntityAt(position, lowerLevel, threshold)) return this.m_position;

    position = vec2.sub(position, this.transform.position.value);

    if (lowerLevel && this.m_left && this.m_left.getEntityAt(position, lowerLevel, threshold))
      return this.m_left;

    if (lowerLevel && this.m_right && this.m_right.getEntityAt(position, lowerLevel, threshold))
      return this.m_right;

    return undefined;
  }

  getEntitiesIn(
    box: Box,
    entities: Set<Entity>,
    lowerLevel: boolean | undefined = true,
    angle: number = 0,
    center?: vec2
  ): void {
    if (angle !== 0 && center) {
      if (isPointInBox(vec2.rotate(this.transform.position.value, center, angle), box))
        entities.add(this);
      return;
    }

    if (isPointInBox(this.transform.position.value, box)) entities.add(this);
  }

  getDrawable(): Drawable {
    return { operations: [] };
  }

  getOutlineDrawable = this.getDrawable;

  render(): void {}

  asObject(duplicate: boolean = false): VertexObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: this.transform.position.value,
      left: this.transform.left?.value,
      right: this.transform.right?.value
    };
  }

  toJSON(): VertexObject {
    return this.asObject(false);
  }
}

export default Vertex;
