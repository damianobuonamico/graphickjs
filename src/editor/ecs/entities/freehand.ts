import WobbleSmoother from '@/editor/freehand/wobbleSmoother';
import InputManager from '@/editor/input';
import { Renderer } from '@/editor/renderer';
import {
  average,
  closestPointToLine,
  doesBoxIntersectBox,
  doesBoxIntersectRotatedBox,
  getLineCircleIntersections,
  isPointInBox,
  isPointInCircle,
  lerp,
  vec2
} from '@/math';
import { nanoid } from 'nanoid';
import { Cache } from '../components/cache';
import LayerCompositing from '../components/layerCompositing';
import { FreehandTransform, SimpleTransform } from '../components/transform';
import { getStroke } from 'perfect-freehand';
import CommandHistory from '@/editor/history/history';
import { ChangeCommand } from '@/editor/history/command';
import { getFreehandGeometry } from '@/editor/freehand/strokeBuilder';
import SceneManager from '@/editor/scene';

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
    this.m_cache = new Cache();
    this.transform = new FreehandTransform(this, this.m_cache, position, rotation);
    this.layer = new LayerCompositing();

    this.points = points;

    this.color = color;
  }

  set points(points: vec3[]) {
    this.m_points = points.map((point) => [new SimpleTransform([point[0], point[1]]), point[2]]);
    this.m_cache.pause = true;
  }

  private onGeometryCacheMiss(): [Float32Array, number[]] {
    const position = this.transform.position.value;
    return getFreehandGeometry(
      this.m_points.map((point) => {
        const pos = point[0].position.value;
        return [pos[0] + position[0], pos[1] + position[1], point[1]];
      }),
      SceneManager.viewport.zoom,
      {
        size: 2,
        thinning: 0.6,
        smoothing: 0.5,
        streamline: 0.1
      }
    );
  }

  get geometry() {
    // TODO: don't recalculate at each zoom level (maybe randomize it?)
    return this.m_cache.cached(
      `geometry-${SceneManager.viewport.zoom}`,
      this.onGeometryCacheMiss.bind(this),
      'geometry '
    );
  }

  private onGetPath2DDataCacheMiss() {
    // let path = 'M0,0';

    // for (let i = 1, n = this.m_points.length; i < n; ++i) {
    //   const position = this.m_points[i][0].position.value;
    //   path += `L${position[0]},${position[1]}`;
    // }

    // return new Path2D(path);

    const stroke = getStroke(
      this.m_points.map((point) => [...point[0].position.value, point[1]]),
      {
        size: 8,
        simulatePressure: false,
        thinning: 0.6,
        smoothing: 0.5,
        streamline: 0.1,
        last: true
      }
    );

    return new Path2D(this.getSvgPathFromStroke(<vec2[]>stroke));
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
    const corrected = WobbleSmoother.update(point, InputManager.time);

    this.m_points[this.m_points.length - 1][0] = new SimpleTransform(corrected);
    this.m_points.push([new SimpleTransform(point), pressure]);

    this.m_cache.pause = true;
  }

  pp: vec2[] = [];

  // TODO: fix history (and optimize algorithm)
  erase(position: vec2, radius: number): void {
    this.pp.length = 0;
    this.pp.push(position);
    const angle = this.transform.rotation.value;
    const mid = this.transform.center;

    if (angle !== 0) position = vec2.rotate(position, mid, -this.transform.rotation.value);

    position = vec2.sub(position, this.transform.position.value);

    if (
      this.m_points.length === 0 ||
      (this.m_points.length === 1 &&
        isPointInCircle(this.m_points[0][0].position.value, position, radius))
    ) {
      this.parent.remove(this.id);
      return;
    }

    let fragments: vec3[][] = [[]];
    let dangling = false;

    for (let i = 1, n = this.m_points.length; i < n; ++i) {
      const a = this.m_points[i - 1][0].position.value;
      const b = this.m_points[i][0].position.value;

      if (isPointInCircle(a, position, radius) && isPointInCircle(b, position, radius)) continue;

      let box: Box = [vec2.min(a, b), vec2.max(a, b)];

      if (isPointInBox(position, box, radius)) {
        const points = getLineCircleIntersections([a, b], position, radius);

        this.pp.push(
          ...points.map((point) => vec2.add(point.point, this.transform.position.value))
        );

        if (points.length === 1) {
          if (dangling) {
            fragments.push([
              [...points[0].point, lerp(this.m_points[i - 1][1], this.m_points[i][1], points[0].t)]
            ]);
          } else {
            fragments[fragments.length - 1].push([...a, this.m_points[i - 1][1]]);
            fragments[fragments.length - 1].push([
              ...points[0].point,
              lerp(this.m_points[i - 1][1], this.m_points[i][1], points[0].t)
            ]);

            dangling = true;
          }
        } else if (points.length === 2) {
          fragments[fragments.length - 1].push([...a, this.m_points[i - 1][1]]);
          fragments[fragments.length - 1].push([
            ...points[0].point,
            lerp(this.m_points[i - 1][1], this.m_points[i][1], points[0].t)
          ]);
          fragments.push([
            [...points[0].point, lerp(this.m_points[i - 1][1], this.m_points[i][1], points[0].t)]
          ]);
        } else {
          fragments[fragments.length - 1].push([...a, this.m_points[i - 1][1]]);
        }
      } else {
        fragments[fragments.length - 1].push([...a, this.m_points[i - 1][1]]);
      }
    }

    if (
      !isPointInCircle(this.m_points[this.m_points.length - 1][0].position.value, position, radius)
    ) {
      fragments[fragments.length - 1].push([
        ...this.m_points[this.m_points.length - 1][0].position.value,
        this.m_points[this.m_points.length - 1][1]
      ]);
    }

    if (fragments.length === 0) return;
    else if (fragments.length === 1) {
      const backup = this.m_points;
      this.points = fragments[0];
      const points = this.m_points;

      CommandHistory.add(
        new ChangeCommand(
          () => {
            this.m_points = points;
            this.m_cache.pause = true;
            this.transform.keepCentered(mid);
          },
          () => {
            this.m_points = backup;
            this.m_cache.pause = true;
            this.transform.keepCentered(mid);
          }
        )
      );
    } else {
      fragments.forEach((fragment) => {
        const freehand = new Freehand({
          position: this.transform.position.value,
          rotation: this.transform.rotation.value,
          points: fragment
        });
        this.parent.add(freehand);

        freehand.transform.keepCentered(mid);
      });

      this.parent.remove(this.id);
    }
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
      { type: 'fillColor', data: [this.color] }
      // { type: 'strokeWidth', data: [8] }
    ];

    operations.push({ type: 'path2D', data: [this.getPath2DData(), false, true] });

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
    // Renderer.debugPoints(
    //   this.id,
    //   this.m_points
    //     .map((point) => vec2.add(point[0].position.value, this.transform.position.value))
    //     .concat(this.pp)
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
