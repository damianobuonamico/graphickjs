import { Cache, ElementCache } from '@/editor/ecs/components/cache';
import { doesBoxIntersectBox, doesBoxIntersectRotatedBox, isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../../history';
import { Renderer } from '../../renderer';
import SceneManager from '../../scene';
import { ElementSelectionManager } from '../../selection';
import Bezier from './bezier';
import Layer from './layer';
import Vertex from './vertex';
import Fill from '../components/fill';
import Stroke from '../components/stroke';
import { GEOMETRY_MAX_INTERSECTION_ERROR } from '@/utils/constants';
import { ElementTransform } from '../components/transform';
import Debugger from '@/utils/debugger';

class Element implements ElementEntity {
  readonly id: string;
  readonly type: EntityType = 'element';
  readonly selectable = true;
  readonly selection = new ElementSelectionManager(this);

  parent: Layer;
  transform: ElementTransform;
  fill: Fill | null;
  stroke: Stroke | null;

  private m_order: string[] = [];
  private m_vertices: Map<string, VertexEntity> = new Map();
  private m_curves: Map<string, BezierEntity> = new Map();
  private m_closed: boolean;
  private m_recordHistory: boolean;
  private m_fillRule: 'even-odd' | 'non-zero' = 'non-zero';

  private m_cache: ElementCache = new ElementCache();

  constructor({
    id = nanoid(),
    vertices,
    transform,
    position,
    rotation,
    closed = false,
    stroke,
    fill,
    recordHistory = true
  }: ElementOptions) {
    this.id = id;
    this.m_closed = closed;
    this.transform = new ElementTransform(
      this,
      this.m_cache.last,
      transform?.position || position,
      transform?.rotation || rotation
    );
    this.m_recordHistory = recordHistory;

    if (vertices) this.vertices = vertices;

    if (fill) this.fill = new Fill(fill);
    if (stroke) this.stroke = new Stroke(stroke);
  }

  get length(): number {
    return this.m_order.length;
  }

  get last(): VertexEntity {
    return this.m_vertices.get(this.m_order[this.m_order.length - 1])!;
  }

  set vertices(vertices: VertexEntity[]) {
    this.m_order.length = 0;
    this.m_vertices.clear();

    vertices.forEach((vertex) => {
      this.pushVertex(vertex, false);
    });

    this.regenerate();
  }

  get cache() {
    return this.m_cache as any as Cache;
  }

  private onClosingCurveCacheMiss(): Bezier {
    const curves = Array.from(this.m_curves.values());
    return new Bezier({
      start: new Vertex({ position: curves[curves.length - 1].p3 }),
      end: new Vertex({ position: curves[0].p0 })
    });
  }

  private get m_closingCurve(): Bezier {
    Debugger.time('cCurve');
    const bezier = this.m_cache.cached('closingCurve', this.onClosingCurveCacheMiss.bind(this));
    Debugger.timeEnd('cCurve');
    return bezier;
  }

  private pushVertex(
    vertex: VertexEntity,
    regenerate = true,
    index: number = this.m_order.length
  ): void {
    this.m_vertices.set(vertex.id, vertex);
    this.m_order.splice(index, 0, vertex.id);

    vertex.parent = this;

    if (regenerate) this.regenerate();
    else this.m_cache.pause = true;
  }

  private spliceVertex(
    id: string,
    regenerate = true,
    index: number = this.m_order.indexOf(id)
  ): void {
    this.m_vertices.delete(id);
    this.m_order.splice(index, 1);

    if (regenerate) this.regenerate();
    else this.m_cache.pause = true;
  }

  public points: vec2[] = [];

  regenerate(ids: string[] = this.m_order): void {
    const curves = new Map<string, BezierEntity>();
    const vertices = new Map<string, VertexEntity>();

    let last: VertexEntity | null = null;

    ids.forEach((id) => {
      const vertex = this.m_vertices.get(id)!;

      vertices.set(vertex.id, vertex);

      if (last) {
        const bezier = new Bezier({ start: last, end: vertex });
        curves.set(bezier.id, bezier);
        bezier.parent = this;
      }

      last = vertex;
    });

    if (this.m_closed) {
      const bezier = new Bezier({
        start: this.m_vertices.get(ids[ids.length - 1])!,
        end: this.m_vertices.get(ids[0])!
      });

      curves.set(bezier.id, bezier);

      bezier.parent = this;
    }

    this.m_order = [...ids];
    this.m_vertices = vertices;
    this.m_curves = curves;

    this.m_cache.pause = true;
  }

  forEach(callback: (vertex: VertexEntity, selected?: boolean | undefined) => void): void {
    this.m_vertices.forEach((vertex) => callback(vertex, this.selection.has(vertex.id)));
  }

  forEachBezier(callback: (bezier: BezierEntity) => void): void {
    this.m_curves.forEach((curve) => callback(curve));
  }

  reverse(): void {
    HistoryManager.record({
      fn: () => {
        this.regenerate(this.m_order.reverse());
        this.m_vertices.forEach((vertex) => {
          const left = vertex.left;
          const right = vertex.right;

          vertex.left = right;
          vertex.right = left;
        });
      },
      undo: () => {
        this.regenerate(this.m_order.reverse());
        this.m_vertices.forEach((vertex) => {
          const left = vertex.left;
          const right = vertex.right;

          vertex.left = right;
          vertex.right = left;
        });
      }
    });
  }

  concat(element: ElementEntity): void {
    const backup = [...this.m_order];

    const box = this.transform.unrotatedBoundingBox;
    const mid = vec2.mid(
      vec2.sub(box[0], this.transform.position),
      vec2.sub(box[1], this.transform.position)
    );
    const angle = this.transform.rotation;

    const box1 = element.transform.unrotatedBoundingBox;
    const mid1 = vec2.mid(
      vec2.sub(box1[0], element.transform.position),
      vec2.sub(box1[1], element.transform.position)
    );
    const angle1 = element.transform.rotation;

    element.forEach((vertex) => {
      vertex.transform.position = vec2.rotate(
        vec2.add(
          vec2.rotate(vertex.transform.position, mid1, angle1),
          vec2.sub(element.transform.staticPosition, this.transform.staticPosition)
        ),
        mid,
        -angle
      );

      if (vertex.left)
        vertex.transform.left = vec2.rotate(vertex.transform.left, [0, 0], angle1 - angle);
      if (vertex.right)
        vertex.transform.right = vec2.rotate(vertex.transform.right, [0, 0], angle1 - angle);
    });

    HistoryManager.record({
      fn: () => {
        element.forEach((vertex) => {
          this.pushVertex(vertex, false);
        });

        this.regenerate();
        element.parent.delete(element, true);
      },
      undo: () => {
        element.forEach((vertex) => {
          this.spliceVertex(vertex.id, false);
          vertex.parent = element;
        });

        element.parent.add(element, true);
        this.regenerate(backup);
      }
    });

    const box2 = this.transform.unrotatedBoundingBox;
    const mid2 = vec2.mid(
      vec2.sub(box2[0], this.transform.position),
      vec2.sub(box2[1], this.transform.position)
    );

    this.transform.translate(
      vec2.sub(
        vec2.rotate([0, 0], mid, this.transform.rotation),
        vec2.rotate([0, 0], mid2, this.transform.rotation)
      )
    );
  }

  split(bezier: BezierEntity, position: vec2): VertexEntity | void {
    if (!this.m_curves.has(bezier.id)) return;

    if (this.transform.rotation !== 0) {
      const box = this.transform.unrotatedBoundingBox;
      const mid = vec2.mid(box[0], box[1]);
      position = vec2.rotate(position, mid, -this.transform.rotation);
    }

    position = vec2.sub(position, this.transform.position);

    const vertex = bezier.split(position);

    this.push(vertex, true, this.m_order.indexOf(bezier.start.id) + 1);

    return vertex;
  }

  push(vertex: VertexEntity, regenerate?: boolean, index?: number): void {
    HistoryManager.record({
      fn: () => {
        this.pushVertex(vertex, regenerate, index);
      },
      undo: () => {
        this.spliceVertex(vertex.id, regenerate);
      }
    });
  }

  close(mergeThreshold: number = 1e-4): void {
    // Check if there are at least 2 vertices
    if (this.length < 2) return;

    // Check if the element is a line segment
    if (this.m_curves.size === 1 && this.m_curves.values().next().value.bezierType === 'linear')
      return;

    const first = this.m_vertices.get(this.m_order[0])!;
    const last = this.m_vertices.get(this.m_order[this.m_order.length - 1])!;

    if (
      vec2.sqrDist(first.transform.position, last.transform.position) < Math.pow(mergeThreshold, 2)
    ) {
      if (last.left) first.transform.left = last.transform.left;
      this.delete(last, true);
    }

    const box = this.transform.unrotatedBoundingBox;
    const mid = vec2.mid(box[0], box[1]);

    if (this.m_order[0]) {
      HistoryManager.record({
        fn: () => {
          this.m_closed = true;
          this.regenerate();
        },
        undo: () => {
          this.m_closed = false;
          this.regenerate();
        }
      });
    }

    const box1 = this.transform.unrotatedBoundingBox;
    const mid1 = vec2.mid(box1[0], box1[1]);

    this.transform.translate(
      vec2.sub(
        vec2.rotate([0, 0], mid, this.transform.rotation),
        vec2.rotate([0, 0], mid1, this.transform.rotation)
      )
    );
  }

  isOpenEnd(id: string): boolean {
    if (!this.m_closed && (this.m_order[0] === id || this.m_order[this.m_order.length - 1] === id))
      return true;
    return false;
  }

  isFirstVertex(id: string): boolean {
    if (this.m_order[0] === id) return true;
    return false;
  }

  intersects(box: Box): boolean {
    const angle = this.transform.rotation;

    if (angle === 0) {
      if (!doesBoxIntersectBox(box, this.transform.boundingBox)) return false;

      box = [vec2.sub(box[0], this.transform.position), vec2.sub(box[1], this.transform.position)];

      if (
        this.length < 2 &&
        isPointInBox(Array.from(this.m_vertices.values())[0].transform.position, box, 5)
      )
        return true;

      return Array.from(this.m_curves.values()).some((segment) => segment.intersectsBox(box));
    } else {
      const unrotatedBox = this.transform.unrotatedBoundingBox;
      const mid = vec2.mid(unrotatedBox[0], unrotatedBox[1]);

      if (!doesBoxIntersectRotatedBox(box, unrotatedBox, angle)) return false;

      const position = this.transform.position;
      const rotated: vec2[] = [box[0], [box[1][0], box[0][1]], box[1], [box[0][0], box[1][1]]].map(
        (point) => vec2.sub(vec2.rotate(point as vec2, mid, -angle), position)
      );

      rotated.push(rotated[0]);

      const rects: Box[] = [];

      for (let i = 0; i < rotated.length - 1; i++) {
        rects.push([rotated[i], rotated[i + 1]]);
      }

      box = [vec2.sub(box[0], position), vec2.sub(box[1], position)];
      vec2.sub(mid, position, mid);

      return Array.from(this.m_curves.values()).some((segment) => {
        if (isPointInBox(vec2.rotate(segment.p0, mid, angle), box)) return true;
        return rects.some((rect) => segment.intersectsLine(rect));
      });
    }
  }

  delete(vertex: VertexEntity | true, keepClosed = true): void {
    if (this.m_order.length < 3) {
      SceneManager.delete(this, true);
      return;
    }

    const box = this.transform.unrotatedBoundingBox;
    const mid = vec2.mid(box[0], box[1]);

    const vertices = vertex === true ? this.selection.entities : [vertex];
    const indices = vertices.map((vertex) => this.m_order.indexOf(vertex.id)).sort();

    let fragments: string[][] = [];

    for (let i = 0; i < indices.length; i++) {
      let prev = i < 1 ? -1 : indices[i - 1];
      if (prev !== indices[i] - 1) fragments.push(this.m_order.slice(prev + 1, indices[i]));
    }

    if (indices[indices.length - 1] < this.m_order.length - 1) {
      fragments.push(this.m_order.slice(indices[indices.length - 1] + 1));
    }

    if (this.m_closed || keepClosed) {
      if (keepClosed) {
        const order: string[] = [];
        const backup = [...this.m_order];

        fragments.forEach((fragment) => {
          order.push(...fragment);
        });

        const vertices = this.m_order
          .filter((id) => order.indexOf(id) === -1)
          .map((id) => this.m_vertices.get(id));

        HistoryManager.record({
          fn: () => {
            this.regenerate(order);
          },
          undo: () => {
            vertices.forEach((vertex) => {
              if (vertex) this.pushVertex(vertex, false);
            });
            this.selection.restore(vertices as VertexEntity[]);
            this.regenerate(backup);
          }
        });

        return;
      } else {
        if (
          fragments[0][0] === this.m_order[0] &&
          fragments[fragments.length - 1][fragments[fragments.length - 1].length - 1] ===
            this.m_order[this.m_order.length - 1]
        ) {
          const last = fragments.pop();
          if (last) fragments[0] = last.concat(fragments[0]);
        }
      }
    }

    const elements: Element[] = [];

    for (let i = 0; i < fragments.length; i++) {
      fragments[i] = fragments[i].filter((id) => id != undefined);
      if (fragments[i].length > 1) {
        const element = new Element({
          vertices: fragments[i].map((id) => this.m_vertices.get(id)!),
          position: this.transform.staticPosition,
          rotation: this.transform.staticRotation,
          fill: this.fill?.asObject(),
          stroke: this.stroke?.asObject()
        });
        const box1 = element.transform.unrotatedBoundingBox;
        const mid1 = vec2.mid(box1[0], box1[1]);
        element.transform.translate(
          vec2.sub(
            vec2.rotate([0, 0], mid, this.transform.staticRotation),
            vec2.rotate([0, 0], mid1, this.transform.staticRotation)
          )
        );
        SceneManager.add(element);
        elements.push(element);
      }
    }

    SceneManager.delete(this, true);

    HistoryManager.record({
      fn: () => {
        elements.forEach((element) => {
          element.forEach((vertex) => (vertex.parent = element));
        });
        this.selection.clear();
      },
      undo: () => {
        this.forEach((vertex) => (vertex.parent = this));
      }
    });
  }

  destroy(): void {}

  getEntityAt(position: vec2, lowerLevel: boolean, threshold: number): Entity | undefined {
    const angle = this.transform.rotation;
    const box = this.transform.unrotatedBoundingBox;
    const mid = vec2.mid(box[0], box[1]);

    if (angle !== 0) {
      position = vec2.rotate(position, mid, -this.transform.rotation);
    }

    if (
      isPointInBox(
        position,
        lowerLevel ? this.transform.largeBoundingBox : this.transform.unrotatedBoundingBox,
        threshold
      )
    ) {
      position = vec2.sub(position, this.transform.position);

      let toReturn: Entity | undefined = undefined;

      this.m_vertices.forEach((vertex) => {
        if (!toReturn) toReturn = vertex.getEntityAt(position, lowerLevel, threshold) ?? toReturn;
      });

      if (!toReturn) {
        this.m_curves.forEach((bezier) => {
          if (!toReturn) toReturn = bezier.getEntityAt(position, lowerLevel, threshold) ?? toReturn;
        });
      }

      if (this.fill && !toReturn) {
        const rect: Box = [position, [Infinity, position[1]]];

        if (this.m_fillRule === 'even-odd') {
          let intersections = 0;

          this.m_curves.forEach((bezier) => {
            const points = bezier.getLineIntersectionPoints(rect);
            intersections += points.length;
          });

          if (!this.m_closed && this.length > 1)
            intersections += this.m_closingCurve.getLineIntersectionPoints(rect).length;

          if (intersections % 2 !== 0) toReturn = this;
        } else {
          let count = 0;

          const curves = Array.from(this.m_curves.values());
          if (!this.m_closed && this.length > 1) curves.push(this.m_closingCurve);

          curves.forEach((bezier) => {
            bezier.getLineIntersections(rect).forEach((t) => {
              if (bezier.getPoint(t)[0] > position[0]) {
                if (
                  bezier.getPoint(t - GEOMETRY_MAX_INTERSECTION_ERROR)[1] <
                  bezier.getPoint(t + GEOMETRY_MAX_INTERSECTION_ERROR)[1]
                )
                  count++;
                else count--;
              }
            });
          });

          if (count !== 0) toReturn = this;
        }
      }

      return toReturn;
    }

    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean): void {
    if (lowerLevel) {
      const position = this.transform.position;

      box = [vec2.sub(box[0], position), vec2.sub(box[1], position)];
      const unrotatedBox = this.transform.unrotatedBoundingBox;
      const mid = vec2.sub(vec2.mid(unrotatedBox[0], unrotatedBox[1]), position);
      const angle = this.transform.rotation;

      this.m_vertices.forEach((vertex) => {
        vertex.getEntitiesIn(box, entities, true, angle, mid);
      });
    } else if (this.intersects(box)) entities.add(this);
  }

  private onGetDrawableCacheMiss(): Drawable {
    const drawable: Drawable = { operations: [{ type: 'begin' }] };

    let first = true;

    this.m_curves.forEach((bezier) => {
      if (first) {
        drawable.operations.push({ type: 'move', data: [bezier.p0] });
        first = false;
      }

      drawable.operations.push(...bezier.getDrawable().operations);
    });

    if (this.m_closed) drawable.operations.push({ type: 'close' });
    if (this.fill) drawable.operations.push({ type: 'fill' });
    if (this.stroke) drawable.operations.push({ type: 'stroke' });

    return drawable;
  }

  getDrawable(useWebGL: boolean = false): Drawable {
    Debugger.time('drwb');
    const drawable = this.m_cache.cached('drawable', this.onGetDrawableCacheMiss.bind(this));
    Debugger.timeEnd('drwb');
    return drawable;
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    const ops = this.getDrawable(useWebGL).operations.filter(
      (op) =>
        op.type !== 'fill' &&
        op.type !== 'stroke' &&
        op.type !== 'strokecolor' &&
        op.type !== 'fillcolor'
    );

    ops.push({ type: 'stroke' });

    return {
      operations: ops
    };
  }

  render(): void {
    Renderer.element(this);
  }

  asObject(duplicate?: boolean): ElementObject {
    const obj: ElementObject = {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      transform: this.transform.asObject(),
      vertices: this.m_order.map(
        (id) => this.m_vertices.get(id)!.asObject(duplicate) as VertexObject
      )
    };

    if (this.m_closed) obj.closed = true;

    if (this.fill) obj.fill = this.fill.asObject(duplicate);
    if (this.stroke) obj.stroke = this.stroke.asObject(duplicate);

    return obj;
  }

  toJSON(): ElementObject {
    return this.asObject(false);
  }
}

export default Element;
