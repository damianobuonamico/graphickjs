import { Renderer } from '@/editor/renderer';
import {
  average,
  doesBoxIntersectBox,
  doesBoxIntersectRotatedBox,
  isPointInBox,
  vec2
} from '@/math';
import { nanoid } from 'nanoid';
import getStroke from 'perfect-freehand';
import { Cache } from '../components/cache';
import LayerCompositing from '../components/layerCompositing';
import { FreehandTransform, SimpleTransform } from '../components/transform';

export const isFreehand = (b: Entity): b is Freehand => {
  return b.type === 'freehand';
};

class Freehand implements FreehandEntity {
  readonly id: string;
  readonly type = 'freehand';
  readonly selectable = true;
  readonly transform: FreehandTransform;
  readonly layer: LayerCompositingComponent;

  // TEMP
  color: string;

  parent: LayerEntity;

  private m_cache: CacheComponent;
  private m_points: [SimpleTransform, number][];

  constructor({ position, rotation, points, color = '#222' }: FreehandOptions) {
    this.id = nanoid();
    this.transform = new FreehandTransform(this, position, rotation);
    this.layer = new LayerCompositing();

    this.m_cache = new Cache();
    this.points = points;

    this.color = color;
  }

  set points(points: vec3[]) {
    this.m_points = points.map((point) => [new SimpleTransform([point[0], point[1]]), point[2]]);
  }

  private onGetPath2DDataCacheMiss() {
    let path = 'M0,0';

    for (let i = 1, n = this.m_points.length; i < n; ++i) {
      const position = this.m_points[i][0].position.value;
      path += `L${position[0]},${position[1]}`;
    }

    // const len = this.m_points.length;

    // if (len < 3) return new Path2D('M0,0');

    // let a = this.m_points[0][0].position.value;
    // let b = this.m_points[1][0].position.value;
    // const c = this.m_points[2][0].position.value;

    // let path = `M${a[0].toFixed(2)},${a[1].toFixed(2)} Q${b[0].toFixed(2)},${b[1].toFixed(
    //   2
    // )} ${average(b[0], c[0]).toFixed(2)},${average(b[1], c[1]).toFixed(2)} T`;

    // for (let i = 2, max = len - 1; i < max; i++) {
    //   a = this.m_points[i][0].position.value;
    //   b = this.m_points[i + 1][0].position.value;
    //   path += `${average(a[0], b[0]).toFixed(2)},${average(a[1], b[1]).toFixed(2)} `;
    // }

    return new Path2D(path);

    // const stroke = getStroke(
    //   this.m_points.map((point) => [...point[0].position.value, point[1]]),
    //   { size: 8, simulatePressure: false }
    // );

    // return new Path2D(this.getSvgPathFromStroke(<vec2[]>stroke));
  }

  private getPath2DData() {
    return this.m_cache.cached('path2D', this.onGetPath2DDataCacheMiss.bind(this));
  }

  private getSvgPathFromStroke(points: vec2[], closed = true) {
    const len = points.length;

    if (len < 4) {
      return ``;
    }

    let a = points[0];
    let b = points[1];
    const c = points[2];

    let result = `M${a[0].toFixed(2)},${a[1].toFixed(2)} Q${b[0].toFixed(2)},${b[1].toFixed(
      2
    )} ${average(b[0], c[0]).toFixed(2)},${average(b[1], c[1]).toFixed(2)} T`;

    for (let i = 2, max = len - 1; i < max; i++) {
      a = points[i];
      b = points[i + 1];
      result += `${average(a[0], b[0]).toFixed(2)},${average(a[1], b[1]).toFixed(2)} `;
    }

    if (closed) {
      result += 'Z';
    }

    return result;
  }

  forEach(callback: (point: SimpleTransform) => void) {
    this.m_points.forEach((point) => callback(point[0]));
  }

  add(point: vec2, pressure: number = 0.5) {
    this.m_points.push([new SimpleTransform(point), pressure]);
    this.m_cache.pause = true;
  }

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    const box = this.transform.unrotatedBoundingBox;
    const mid = vec2.mid(box[0], box[1]);
    const angle = this.transform.rotation.value;

    if (angle !== 0) position = vec2.rotate(position, mid, -angle);
    if (isPointInBox(position, box, threshold)) return this;

    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel: boolean = false): void {
    const angle = this.transform.rotation.value;

    if (angle !== 0) {
      if (doesBoxIntersectRotatedBox(box, this.transform.unrotatedBoundingBox, angle))
        entities.add(this);
    } else {
      if (doesBoxIntersectBox(box, this.transform.unrotatedBoundingBox)) entities.add(this);
    }
  }

  getDrawable(): Drawable {
    const operations: DrawOp[] = [
      { type: 'strokeColor', data: [this.color] },
      { type: 'strokeWidth', data: [8] }
    ];

    operations.push({ type: 'path2D', data: [this.getPath2DData(), true, false] });

    return { operations };
  }

  getOutlineDrawable(): Drawable {
    const box = this.transform.unrotatedBoundingBox;

    return {
      operations: [
        {
          type: 'rect',
          data: [vec2.sub(box[0], this.transform.position.value), vec2.sub(box[1], box[0])]
        }
      ]
    };
  }
  render(): void {
    Renderer.freehand(this);
    // this.m_points.forEach((point) =>
    //   Renderer.debugRect({
    //     position: vec2.add(this.transform.position.value, point[0].position.value)
    //   })
    // );
  }

  asObject(duplicate: boolean = false): FreehandObject {
    const object: FreehandObject = {
      id: duplicate ? nanoid() : this.id,
      type: 'freehand',
      position: this.transform.position.value,
      points: this.m_points.map((point) => [...point[0].position.value, point[1]])
    };

    if (this.transform.rotation.value !== 0) object.rotation = this.transform.rotation.value;

    return object;
  }

  toJSON(): FreehandObject {
    return this.asObject(false);
  }
}

export default Freehand;
