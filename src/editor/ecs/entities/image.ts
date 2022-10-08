import HistoryManager from '@/editor/history';
import { doesBoxIntersectBox, doesBoxIntersectRotatedBox, isPointInBox, vec2 } from '@/math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import { Transform, TransformVec2Value } from '../components/transform';
import Layer from './layer';

class ImageMedia implements ImageEntity {
  readonly id: string;
  readonly type: EntityType = 'image';
  readonly selectable = true;

  parent: Layer;
  transform: TransformComponent;

  private m_data: string;
  private m_source = new Image();
  private m_size: TransformVec2Value = new TransformVec2Value();
  public reflect: [boolean, boolean] = [false, false];
  private wasLastNegative: [boolean, boolean] = [false, false];

  constructor({ id = nanoid(), source, position, size }: ImageOptions) {
    this.id = id;
    this.transform = new Transform(position, undefined, (magnitude, origin, temp, apply) =>
      this.scale(magnitude, origin, temp, apply)
    );

    this.m_data = source;
    this.m_source.src = source;

    this.m_source.onload = () => {
      HistoryManager.skipNext();
      if (size && !vec2.equals(size, vec2.create())) this.m_size.set(size);
      else this.m_size.set([this.m_source.width, this.m_source.height]);
    };
  }

  get size(): vec2 {
    return this.m_size.get();
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
    return [
      this.transform.staticPosition,
      vec2.add(this.transform.staticPosition, this.m_size.staticGet())
    ];
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
    return [this.transform.position, vec2.add(this.transform.position, this.m_size.get())];
  }

  private scale(
    magnitude: vec2,
    origin: vec2 = this.transform.origin,
    temp = false,
    apply?: boolean
  ) {
    if (apply === true) {
      this.m_size.apply();
      this.wasLastNegative = [false, false];
      return;
    } else if (apply === false) {
      this.m_size.clear();
      this.wasLastNegative = [false, false];
      return;
    }

    origin = vec2.add(origin, this.transform.position);

    const box = this.staticBoundingBox;
    const scaled = [vec2.scale(box[0], origin, magnitude), vec2.scale(box[1], origin, magnitude)];
    const size = vec2.sub(scaled[1], scaled[0]);

    if (size[0] < 0) {
      if (!this.wasLastNegative[0]) this.reflect[0] = !this.reflect[0];
      this.wasLastNegative[0] = true;
    } else {
      if (this.wasLastNegative[0]) this.reflect[0] = !this.reflect[0];
      this.wasLastNegative[0] = false;
    }
    if (size[1] < 0) {
      if (!this.wasLastNegative[1]) this.reflect[1] = !this.reflect[1];
      this.wasLastNegative[1] = true;
    } else {
      if (this.wasLastNegative[1]) this.reflect[1] = !this.reflect[1];
      this.wasLastNegative[1] = false;
    }

    const position = vec2.min(scaled[0], scaled[1]);

    if (temp) {
      this.transform.tempPosition = position;
      this.m_size.translate(vec2.sub(vec2.abs(size), this.m_size.get()));
    } else {
      this.transform.position = position;
      this.m_size.set(vec2.abs(size));
    }
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
      operations: [{ type: 'rect', data: [[0, 0], this.m_size.get()] }, { type: 'stroke' }]
    };
  }

  getOutlineDrawable(useWebGL = false): Drawable {
    return {
      operations: [{ type: 'rect', data: [[0, 0], this.m_size.get()] }, { type: 'stroke' }]
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
