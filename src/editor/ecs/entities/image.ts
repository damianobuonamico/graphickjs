import HistoryManager from '@/editor/history/history';
import { doesBoxIntersectBox, doesBoxIntersectRotatedBox, isPointInBox, vec2 } from '@/math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import { RectTransform } from '../components/transform';
import Layer from './layer';

export const isImage = (b: Entity): b is ImageEntity => {
  return b.type === 'image';
};

class ImageMedia implements ImageEntity {
  readonly id: string;
  readonly type: EntityType = 'image';
  readonly selectable = true;

  parent: Layer;
  transform: RectTransformComponent;

  private m_data: string;
  private m_source = new Image();

  constructor({ id = nanoid(), source, position, transform, size }: ImageOptions) {
    this.id = id;
    this.transform = new RectTransform(
      transform?.position || position,
      transform?.rotation,
      size,
      transform?.reflection
    );

    this.m_data = source;
    this.m_source.src = source;

    this.m_source.onload = () => {
      HistoryManager.skipNext();
      if (size && !vec2.equals(size, vec2.create())) {
        this.transform.size = size;
      } else {
        this.transform.size = [this.m_source.width, this.m_source.height];
      }
      HistoryManager.clearSkip();
    };
  }

  get size(): vec2 {
    return this.transform.size;
  }

  get source(): HTMLImageElement {
    return this.m_source;
  }

  destroy(): void {}

  getEntityAt(position: vec2, lowerLevel: boolean, threshold: number): Entity | undefined {
    const box = this.transform.unrotatedBoundingBox;
    const mid = vec2.mid(box[0], box[1]);
    if (this.transform.rotation !== 0)
      position = vec2.rotate(position, mid, -this.transform.rotation);
    if (isPointInBox(position, box, threshold)) return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {
    const angle = this.transform.rotation;
    if (angle !== 0) {
      if (doesBoxIntersectRotatedBox(box, this.transform.unrotatedBoundingBox, angle))
        entities.add(this);
    } else {
      if (doesBoxIntersectBox(box, this.transform.unrotatedBoundingBox)) entities.add(this);
    }
  }

  getDrawable = this.getOutlineDrawable;

  getOutlineDrawable(): Drawable {
    const box = this.transform.staticBoundingBox;

    return {
      operations: [
        {
          type: 'rect',
          data: [vec2.sub(box[0], this.transform.staticPosition), vec2.sub(box[1], box[0])]
        }
      ]
    };
  }

  render(): void {
    Renderer.image(this);
  }

  asObject(duplicate: boolean = false): ImageObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      transform: this.transform.asObject(),
      size: this.size,
      source: this.m_data
    };
  }

  toJSON() {
    return this.asObject(false);
  }
}

export default ImageMedia;
