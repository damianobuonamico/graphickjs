import { Cache, VertexCache } from '@/editor/ecs/components/cache';
import { isPointInBox, vec2, vec4 } from '@math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import SceneManager from '../../scene';
import { VertexTransform } from '../components/transform';
import Handle from './handle';

class Vertex implements VertexEntity {
  readonly id: string;
  readonly type: EntityType = 'vertex';
  readonly selectable = false;

  private m_parent: ElementEntity;
  transform: VertexTransformComponent;

  private m_position: HandleEntity;
  private m_left?: HandleEntity;
  private m_right?: HandleEntity;

  private m_cache: VertexCache = new VertexCache();

  constructor({ id = nanoid(), position, left, right }: VertexOptions) {
    this.id = id;
    this.m_position = new Handle({ position, type: 'vertex', parent: this });
    this.m_position.setCache(this.m_cache as any as Cache);

    if (left) this.left = new Handle({ position: left, type: 'bezier', parent: this });
    if (right) this.right = new Handle({ position: right, type: 'bezier', parent: this });

    this.transform = new VertexTransform(this);
  }

  get parent() {
    return this.m_parent;
  }

  set parent(parent: ElementEntity) {
    this.m_parent = parent;
    this.m_cache.parentCache = parent.cache as Cache;
  }

  get position(): HandleEntity {
    return this.m_position;
  }

  get left(): HandleEntity | undefined {
    return this.m_left;
  }

  set left(handle: HandleEntity | undefined) {
    this.m_left = handle;
    handle?.setCache(this.m_cache as any as Cache);
    this.m_cache.pause = true;
  }

  get right(): HandleEntity | undefined {
    return this.m_right;
  }

  set right(handle: HandleEntity | undefined) {
    this.m_right = handle;
    handle?.setCache(this.m_cache as any as Cache);
    this.m_cache.pause = true;
  }

  get boundingBox(): Box {
    let min: vec2 = [0, 0];
    let max: vec2 = [0, 0];

    if (this.m_left) {
      vec2.min(min, this.transform.left, min);
      vec2.max(max, this.transform.left, max);
    }

    if (this.m_right) {
      vec2.min(min, this.transform.right, min);
      vec2.max(max, this.transform.right, max);
    }

    return [vec2.add(min, this.transform.position), vec2.add(max, this.transform.position)] as Box;
  }

  registerCache(cache: Cache): void {
    this.m_cache.register(cache);
  }

  destroy(): void {}

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (this.m_position.getEntityAt(position, lowerLevel, threshold)) return this.m_position;

    position = vec2.sub(position, this.transform.position);

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
      if (isPointInBox(vec2.rotate(this.transform.position, center, angle), box))
        entities.add(this);
      return;
    }

    if (isPointInBox(this.transform.position, box)) entities.add(this);
  }

  getDrawable(useWebGL: boolean = false, selected: boolean = false): Drawable {
    // TODO: optimize operations batching by color
    if (useWebGL) {
      return { operations: [{ type: 'geometry' }] };
    } else {
      const drawable: Drawable = { operations: [{ type: 'begin' }] };
      if (selected) {
        if (this.m_left) {
          drawable.operations.push({
            type: 'circle',
            data: [
              vec2.add(this.m_left.transform.position, this.transform.position),
              3 / SceneManager.viewport.zoom
            ]
          });
        }

        if (this.m_right) {
          drawable.operations.push({
            type: 'circle',
            data: [
              vec2.add(this.m_right.transform.position, this.transform.position),
              3 / SceneManager.viewport.zoom
            ]
          });
        }

        drawable.operations.push({
          type: 'crect',
          data: [this.transform.position, 4 / SceneManager.viewport.zoom]
        });

        drawable.operations.push({ type: 'fill' });
        drawable.operations.push({ type: 'begin' });

        if (this.m_left) {
          drawable.operations.push({
            type: 'move',
            data: [vec2.add(this.m_left.transform.position, this.transform.position)]
          });
          drawable.operations.push({
            type: 'linear',
            data: [this.transform.position]
          });
        }

        if (this.m_right) {
          drawable.operations.push({
            type: 'move',
            data: [vec2.add(this.m_right.transform.position, this.transform.position)]
          });
          drawable.operations.push({
            type: 'linear',
            data: [this.transform.position]
          });
        }
      } else {
        drawable.operations.push({
          type: 'crect',
          data: [this.transform.position, 3 / SceneManager.viewport.zoom]
        });
        drawable.operations.push({ type: 'fillcolor', data: vec4.fromValues(1, 1, 1, 1) });
        drawable.operations.push({ type: 'fill' });
        drawable.operations.push({
          type: 'fillcolor',
          data: vec4.fromValues(49 / 255, 239 / 255, 284 / 255, 1)
        });
      }

      drawable.operations.push({ type: 'stroke' });
      return drawable;
    }
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  render(selected: boolean = false): void {
    Renderer.draw(this.getDrawable(false, !!selected));
  }

  asObject(duplicate: boolean = false): VertexObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: this.transform.position,
      left: this.m_left ? this.transform.left : undefined,
      right: this.m_right ? this.transform.right : undefined
    };
  }

  toJSON(): VertexObject {
    return this.asObject(false);
  }
}

export default Vertex;
