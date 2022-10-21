import HistoryManager from '@/editor/history';
import { doesBoxIntersectBox, doesBoxIntersectRotatedBox, isPointInBox, vec2 } from '@/math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import { Transform, TransformVec2Value } from '../components/transform';
import { RectTransform } from '../components/transforms';
import Layer from './layer';

class ImageMedia implements ImageEntity {
  readonly id: string;
  readonly type: EntityType = 'image';
  readonly selectable = true;

  parent: Layer;
  transform: TransformComponent;
  transforms: RectTransform;

  private m_data: string;
  private m_source = new Image();
  private m_size: TransformVec2Value = new TransformVec2Value();
  private m_reflect: TransformVec2Value = new TransformVec2Value([0, 0]);

  constructor({ id = nanoid(), source, position, size }: ImageOptions) {
    this.id = id;
    // this.transform = new Transform(position, undefined, (magnitude, origin, temp, apply) =>
    //   this.scale(magnitude, origin, temp, apply)
    // );
    this.transform = new RectTransform(position, 0, size);

    this.m_data = source;
    this.m_source.src = source;

    this.m_source.onload = () => {
      HistoryManager.skipNext();
      if (size && !vec2.equals(size, vec2.create())) {
        this.m_size.set(size);
      } else {
        this.m_size.set([this.m_source.width, this.m_source.height]);
      }
    };
  }

  get size(): vec2 {
    return this.m_size.get();
  }

  get source(): HTMLImageElement {
    return this.m_source;
  }

  get boundingBox(): Box {
    return this.transform.boundingBox;
    if (this.transform.rotation === 0) return this.unrotatedBoundingBox;

    const vertices = this.rotatedBoundingBox;

    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    vertices.forEach((vertex) => {
      vec2.min(min, vertex, min);
      vec2.max(max, vertex, max);
    });

    return [min, max];
  }

  get staticBoundingBox(): Box {
    return this.transform.staticBoundingBox;
    return [
      this.transform.staticPosition,
      vec2.add(this.transform.staticPosition, this.m_size.staticGet())
    ];
  }

  get rotatedBoundingBox(): [vec2, vec2, vec2, vec2] {
    return this.transform.rotatedBoundingBox;
    const box = this.unrotatedBoundingBox;
    const angle = this.transform.rotation;
    const center = vec2.mid(box[0], box[1]);
    return [
      vec2.rotate(box[0], center, angle),
      vec2.rotate([box[1][0], box[0][1]], center, angle),
      vec2.rotate(box[1], center, angle),
      vec2.rotate([box[0][0], box[1][1]], center, angle)
    ];
  }

  get unrotatedBoundingBox(): Box {
    return this.transform.unrotatedBoundingBox;
    const origin = vec2.add(this.transform.origin, this.transform.position);
    const box = [
      vec2.scale(this.transform.position, origin, this.magnitude),
      vec2.scale(vec2.add(this.transform.position, this.m_size.get()), origin, this.magnitude)
    ];
    return [vec2.min(box[0], box[1]), vec2.max(box[0], box[1])];
  }

  public points: vec2[] = [];

  private m_magnitude: vec2 = [1, 1];
  public get magnitude() {
    return this.m_magnitude;
  }

  public get reflect() {
    return this.m_reflect.get();
  }

  private scale(
    magnitude: vec2,
    origin: vec2 = this.transform.origin,
    temp = false,
    apply?: boolean
  ) {
    if (apply === true) {
      const box = this.unrotatedBoundingBox;
      this.transform.position = box[0];
      this.m_size.set(vec2.sub(box[1], box[0]));

      const reflect = this.m_reflect.get();
      if (this.magnitude[0] < 0) reflect[0] = reflect[0] === 0 ? 1 : 0;
      if (this.magnitude[1] < 0) reflect[1] = reflect[1] === 0 ? 1 : 0;
      this.m_reflect.set(reflect);

      this.m_magnitude = [1, 1];
      return;
    } else if (apply === false) {
      this.m_magnitude = [1, 1];
      return;
    }

    if (temp) {
      this.m_magnitude = magnitude;
    } else {
      this.m_magnitude = magnitude;
      this.scale([0, 0], undefined, false, true);
    }
  }

  destroy(): void {}

  getEntityAt(position: vec2, lowerLevel: boolean, threshold: number): Entity | undefined {
    const box = this.unrotatedBoundingBox;
    const mid = vec2.mid(box[0], box[1]);
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
    const box = this.unrotatedBoundingBox;

    return {
      operations: [
        {
          type: 'rect',
          data: [vec2.sub(box[0], this.transform.position), vec2.sub(box[1], box[0])]
        },
        { type: 'stroke' }
      ]
    };
  }

  getOutlineDrawable(useWebGL = false): Drawable {
    const box = this.unrotatedBoundingBox;

    return {
      operations: [
        {
          type: 'rect',
          data: [vec2.sub(box[0], this.transform.staticPosition), vec2.sub(box[1], box[0])]
        },
        { type: 'stroke' }
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
