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
import { getStroke, getStrokeOutlinePoints, getStrokePoints } from 'perfect-freehand';
import CommandHistory from '@/editor/history/history';
import { ChangeCommand } from '@/editor/history/command';
import { getFreehandGeometry } from '@/editor/freehand/strokeBuilderOldNew';
import SceneManager from '@/editor/scene';
import { getFreehandStrokeGeometry, getPointsGeometry } from '@/editor/freehand/strokeBuilder';
import { BLI_polyfill_calc } from '@/editor/freehand/polyfill2d';
import {
  bGPDspoint,
  BKE_gpencil_stroke_fill_triangulate,
  BKE_gpencil_stroke_new,
  gpencil_stroke_perimeter_ex
} from '@/editor/freehand/freehandGeometry';
import earcut from 'earcut';
import Tesselator from '@/editor/freehand/tesselator';
import { triangulateStroke } from '@/editor/freehand/stroker';

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

    // let data1: vec3[] = [
    //   [0.9747, 0.1745, 1.0],
    //   [0.9706, 0.177, 1.0],
    //   [0.954, 0.1852, 1.0],
    //   [0.9377, 0.1907, 1.0],
    //   [0.9216, 0.1936, 1.0],
    //   [0.906, 0.1941, 1.0],
    //   [0.8908, 0.1923, 1.0],
    //   [0.8762, 0.1885, 1.0],
    //   [0.8622, 0.1827, 1.0],
    //   [0.849, 0.1752, 1.0],
    //   [0.8365, 0.1662, 1.0],
    //   [0.8249, 0.1558, 1.0],
    //   [0.8143, 0.1441, 1.0],
    //   [0.8046, 0.1314, 1.0],
    //   [0.7961, 0.1179, 1.0],
    //   [0.7888, 0.1036, 1.0],
    //   [0.7828, 0.0889, 1.0],
    //   [0.7802, 0.0813, 1.0]
    // ];

    // const data1: vec3[] = [];

    // for (let i = 0, n = data0.length; i < n; i += 3) {
    //   data1.push([data0[i] + position[0], data0[i + 1] + position[1], data0[i + 2]]);
    // }

    // const data0: vec2[] = data1.map((point) => [
    //   point[0] * 100 + position[0],
    //   point[1] * 100 + position[1]
    //   // point[2]
    // ]);

    // const pm = gpencil_stroke_perimeter_ex(data1, 4, 1, [0, 0]);
    // const tris = BLI_polyfill_calc(pm);
    // const coords = pm.flat();

    // const data0flat = data0.flat();

    // return [Float32Array.from(coords), tris.flat()];
    // return [Float32Array.from(data0flat), data0flat.map((_, i) => i)];
    // return getPointsGeometry(data0, 0.2);

    // const points: vec3[] = this.m_points.map((point) => {
    //   const pos = point[0].position.value;
    //   return [pos[0] + position[0], pos[1] + position[1], point[1]];
    // });

    // const perimeter = gpencil_stroke_perimeter_ex(points, 4, 1, [0, 0]);

    // const tesselated = Tesselator.tesselate(perimeter)[0];
    // const toVec2 = new Float32Array((tesselated.length / 3) * 2);
    // let current = 0;
    // for (let i = 0, n = tesselated.length; i < n; i += 3) {
    //   toVec2[current] = tesselated[i];
    //   toVec2[current + 1] = tesselated[i + 1];
    //   current += 2;
    // }

    // const calculatedIndices = new Array(toVec2.length / 2).fill(0).map((value, index) => index);

    // console.log(calculatedIndices);
    // const tris: vec3[] = [];

    // const tris = BLI_polyfill_calc(perimeter);
    // const coords = perimeter.flat();

    const points: StrokerPoint[] = this.m_points.map((point) => {
      const pos = point[0].position.value;
      return { position: [pos[0] + position[0], pos[1] + position[1]], pressure: point[1] };
    });

    return triangulateStroke(points, 4, SceneManager.viewport.zoom);

    // return [Float32Array.from(coords), earcut(coords)];

    // return [toVec2, calculatedIndices];
    // return getPointsGeometry(points, 0.2);

    // const pts = getStrokeOutlinePoints(
    //   getStrokePoints(
    //     this.m_points.map((point) => {
    //       const pos = point[0].position.value;
    //       return [pos[0] + position[0], pos[1] + position[1], point[1]];
    //     }),
    //     {
    //       size: 2,
    //       thinning: 0.6,
    //       smoothing: 0.5,
    //       streamline: 0.1
    //     }
    //   ),
    //   {
    //     size: 2,
    //     thinning: 0.6,
    //     smoothing: 0.5,
    //     streamline: 0.1
    //   }
    // );

    // const points: vec2[] = [
    //   [4, -3],
    //   [3, -4],
    //   [4, -4],
    //   [5, -5],
    //   [4, -6],
    //   [2, -5],
    //   [2, -6],
    //   [3, -7],
    //   [4, -7],
    //   [6, -6],
    //   [7, -5],
    //   [6, -4],
    //   [6, -3]
    // ];

    const points3d: bGPDspoint[] = [];

    this.m_points.forEach((point) => {
      const pos = point[0].position.value;
      const x = pos[0] + position[0];
      const y = pos[1] + position[1];

      if (points3d[points3d.length - 1]?.x !== x || points3d[points3d.length - 1]?.y !== y)
        points3d.push({
          x,
          y,
          z: 1,
          pressure: point[1],
          time: 0,
          uv_fac: 0,
          uv_fill: [0, 0],
          uv_rot: 0,
          vert_color: [0, 0, 0, 1]
        });
    });

    // const points: vec2[] = [
    //   [0, 0],
    //   [10, 0],
    //   [10, 10],
    //   [0, 10]
    // ];

    // points.forEach((point) => vec2.add(point, position, point));

    // const tris: vec3[] = [];
    // BLI_polyfill_calc(points, points.length, 0, tris);

    // console.log(tris);

    // const stroke = BKE_gpencil_stroke_new(0, points3d.length, 4);

    // stroke.points = points3d;
    // stroke.totpoints = points3d.length;

    // const pts = BKE_gpencil_stroke_fill_triangulate(stroke);

    // return [
    //   Float32Array.from(pts.map((pt) => vec2.add(pt, position, pt)).flat()),
    //   stroke.triangles!.flat()
    // ];
    // return getPointsGeometry(points, 0.1);

    // return getFreehandStrokeGeometry(
    //   this.m_points.map((point) => {
    //     const pos = point[0].position.value;
    //     return [pos[0] + position[0], pos[1] + position[1], point[1]];
    //   }),
    //   SceneManager.viewport.zoom,
    //   { width: 2 }
    // );
    // return getFreehandGeometry(
    //   this.m_points.map((point) => {
    //     const pos = point[0].position.value;
    //     return [pos[0] + position[0], pos[1] + position[1], point[1]];
    //   }),
    //   SceneManager.viewport.zoom,
    //   {
    //     size: 2,
    //     thinning: 0.6,
    //     smoothing: 0.5,
    //     streamline: 0.1
    //   }
    // );
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
