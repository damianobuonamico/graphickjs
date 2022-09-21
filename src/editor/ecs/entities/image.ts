import { doesBoxIntersectBox, isPointInBox, vec2 } from '@/math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import { Transform } from '../components/transform';
import Layer from './layer';

class ImageMedia implements ImageEntity {
  readonly id: string;
  readonly type: EntityType = 'image';

  parent: Layer;
  transform: TransformComponent;

  private m_data: string;
  private m_source = new Image();
  private m_size: vec2;

  constructor({ id = nanoid(), source, position, size }: ImageOptions) {
    this.id = id;
    this.transform = new Transform(position);

    this.m_data = source;
    this.m_source.src = source;

    this.m_source.onload = () => {
      this.m_size =
        size && !vec2.equals(size, vec2.create())
          ? size
          : vec2.fromValues(this.m_source.width, this.m_source.height);
    };
  }

  get size(): vec2 {
    return vec2.clone(this.m_size);
  }

  get source(): HTMLImageElement {
    return this.m_source;
  }

  get boundingBox(): Box {
    return [this.transform.position, vec2.add(this.transform.position, this.m_size)];
  }

  destroy(): void {}

  getEntityAt(position: vec2, lowerLevel: boolean, threshold: number): Entity | undefined {
    if (isPointInBox(position, this.boundingBox, threshold)) return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {
    if (doesBoxIntersectBox(box, this.boundingBox)) entities.add(this);
  }

  getDrawable(useWebGL = false): Drawable {
    // TODO: refactor rendering
    return {
      operations: [{ type: 'rect', data: [[0, 0], this.m_size] }, { type: 'stroke' }]
    };
  }

  getOutlineDrawable(useWebGL = false): Drawable {
    return {
      operations: [{ type: 'rect', data: [[0, 0], this.m_size] }, { type: 'stroke' }]
    };
  }

  render(): void {
    Renderer.image(this);
  }

  asObject(duplicate: boolean = false): ImageObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: this.transform.position,
      size: this.size,
      source: this.m_data
    };
  }

  toJSON() {
    return this.asObject(false);
  }
}

export default ImageMedia;
