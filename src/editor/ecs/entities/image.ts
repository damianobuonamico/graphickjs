import { doesBoxIntersectBox, doesBoxIntersectRotatedBox, isPointInBox, vec2 } from '@/math';
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
  private m_size: vec2 = vec2.create();

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
    if (this.transform.rotation === 0) return this.unrotatedBoundingBox;

    const vertices = this.rotatedBoundingBox;

    let min = [Infinity, Infinity];
    let max = [-Infinity, -Infinity];

    vertices.forEach((vertex) => {
      vec2.min(min, vertex, true);
      vec2.max(max, vertex, true);
    });

    return [min, max];
  }

  get staticBoundingBox(): Box {
    return [this.transform.staticPosition, vec2.add(this.transform.staticPosition, this.m_size)];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    const box = this.unrotatedBoundingBox;
    const angle = this.transform.rotation;
    const center = vec2.div(vec2.add(box[0], box[1]), 2);
    return [
      vec2.rotate(box[0], center, angle),
      vec2.rotate([box[1][0], box[0][1]], center, angle),
      vec2.rotate(box[1], center, angle),
      vec2.rotate([box[0][0], box[1][1]], center, angle)
    ];
  }

  get unrotatedBoundingBox(): Box {
    return [this.transform.position, vec2.add(this.transform.position, this.m_size)];
  }

  destroy(): void {}

  getEntityAt(position: vec2, lowerLevel: boolean, threshold: number): Entity | undefined {
    const box = this.unrotatedBoundingBox;
    const mid = vec2.div(vec2.add(box[0], box[1]), 2);
    if (this.transform.rotation !== 0)
      position = vec2.rotate(position, mid, -this.transform.rotation);
    if (isPointInBox(position, box, threshold)) return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {
    if (this.transform.rotation !== 0) {
      if (doesBoxIntersectRotatedBox(box, this.unrotatedBoundingBox, this.transform.rotation))
        entities.add(this);
    } else {
      if (doesBoxIntersectBox(box, this.unrotatedBoundingBox)) entities.add(this);
    }
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
