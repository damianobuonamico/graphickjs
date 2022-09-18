import { doesBoxIntersectsBox as doesBoxIntersectBox, isPointInBox, vec2 } from '@/math';
import { nanoid } from 'nanoid';
import HistoryManager from '../history';
import { Renderer } from '../renderer';
import Transform from './components/transform';
import Layer from './layer';

class ImageEntity implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'image';

  public parent: Layer;

  private m_data: string;
  private m_source = new Image();
  private m_position: vec2;
  private m_size: vec2;
  private m_transform: Transform;

  constructor({ id = nanoid(), source, position = vec2.create(), size }: ImageOptions) {
    this.id = id;

    this.m_data = source;
    this.m_source.src = source;
    this.m_position = position;
    this.m_transform = new Transform();

    this.m_source.onload = () => {
      this.m_size =
        size && !vec2.equals(size, vec2.create())
          ? size
          : vec2.fromValues(this.m_source.width, this.m_source.height);
    };
  }

  public get size() {
    return this.m_size;
  }

  public get source() {
    return this.m_source;
  }

  public get visible() {
    return true;
  }

  public get position() {
    return this.m_position;
  }

  public set position(position: vec2) {
    this.m_position = vec2.clone(position);
  }

  public get boundingBox(): Box {
    return [this.m_position, vec2.add(this.m_position, this.m_size)];
  }

  public get transform() {
    return this.m_transform.mat4;
  }

  public move(delta: vec2) {
    HistoryManager.record({
      fn: () => {
        vec2.add(this.m_position, delta, true);
      },
      undo: () => {
        vec2.sub(this.m_position, delta, true);
      }
    });
  }

  public moveTo(position: vec2) {
    const backup = vec2.clone(this.m_position);

    HistoryManager.record({
      fn: () => {
        this.position = position;
      },
      undo: () => {
        this.m_position = backup;
      }
    });
  }

  public translate(delta: vec2) {
    this.m_transform.translate(delta);
  }

  public applyTransform() {
    const backup = vec2.clone(this.m_position);
    const transformed = vec2.transformMat4(this.m_position, this.m_transform.mat4);

    if (!vec2.equals(transformed, backup)) {
      HistoryManager.record({
        fn: () => {
          this.m_position = transformed;
        },
        undo: () => {
          this.m_position = backup;
        }
      });

      this.m_transform.clear();
    }
  }

  public clearTransform() {
    this.m_transform.clear();
  }

  public getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    if (isPointInBox(position, this.boundingBox, threshold)) {
      return this;
    }
    return undefined;
  }

  public getEntitiesIn(box: Box, entities: Set<Entity>) {
    if (doesBoxIntersectBox(box, this.boundingBox)) entities.add(this);
  }

  public delete() {}

  public deleteSelf() {}

  public getOutlineDrawable(useWebGL = false): Drawable {
    return {
      operations: [{ type: 'rect', data: [[0, 0], this.m_size] }, { type: 'stroke' }]
    };
  }

  public render() {
    Renderer.image(this);
  }

  public toJSON(duplicate = false) {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: vec2.clone(this.m_position),
      size: vec2.clone(this.m_size),
      source: this.m_data
    } as ImageObject;
  }
}

export default ImageEntity;
