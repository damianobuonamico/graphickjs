import { Cache, ElementCache } from '@/editor/ecs/components/cache';
import { doesBoxIntersectBox, doesBoxIntersectRotatedBox, isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import CommandHistory from '../../history/history';
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
import { BooleanValue, OrderedMapValue } from '@/editor/history/value';
import { ChangeCommand, FunctionCallCommand } from '@/editor/history/command';

export const isElement = (b: Entity): b is Element => {
  return b.type === 'element';
};

class Element implements ElementEntity {
  readonly id: string;
  readonly type = 'element';
  readonly selectable = true;
  readonly selection = new ElementSelectionManager(this);

  readonly transform: ElementTransformComponent;
  readonly fill: Fill | null;
  readonly stroke: Stroke | null;

  parent: Layer;
  opacity = 1;

  private m_vertices: OrderedMapValue<string, VertexEntity> = new OrderedMapValue(
    undefined,
    (vertex) => (vertex.parent = this)
  );
  private m_curves: Map<string, BezierEntity> = new Map();
  private m_closed: BooleanValue;
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
    this.m_closed = new BooleanValue(closed);
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
    return this.m_vertices.size;
  }

  get first(): VertexEntity | undefined {
    return this.m_vertices.last;
  }

  get last(): VertexEntity | undefined {
    return this.m_vertices.last;
  }

  set vertices(vertices: VertexEntity[]) {
    this.m_vertices.clear();

    vertices.forEach((vertex) => {
      this.m_vertices.set(vertex.id, vertex);
    });

    this.regenerate();
  }

  get cache() {
    return this.m_cache as any as Cache;
  }

  private onClosingCurveCacheMiss(): Bezier {
    const curves = Array.from(this.m_curves.values());

    return new Bezier({
      start: new Vertex({ position: curves[curves.length - 1].p3, disableCache: true }),
      end: new Vertex({ position: curves[0].p0, disableCache: true })
    });
  }

  private get m_closingCurve(): Bezier {
    return this.m_cache.cached('closingCurve', this.onClosingCurveCacheMiss.bind(this));
  }

  public points: vec2[] = [];

  private generateCurves(order: string[] = this.m_vertices.order): void {
    const curves = new Map<string, BezierEntity>();
    let last: VertexEntity | null = null;

    for (let i = 0, n = order.length; i < n; ++i) {
      const vertex = this.m_vertices.get(order[i])!;

      if (last) {
        const bezier = new Bezier({ start: last, end: vertex });
        curves.set(bezier.id, bezier);
        bezier.parent = this;
      }

      last = vertex;
    }

    if (this.m_closed.value) {
      const start = this.m_vertices.last;
      const end = this.m_vertices.first;

      if (start && end) {
        const bezier = new Bezier({ start, end });
        curves.set(bezier.id, bezier);
        bezier.parent = this;
      }
    }

    this.m_curves = curves;
    this.m_cache.pause = true;
  }

  regenerate(ids: string[] = this.m_vertices.order): void {
    this.m_vertices.reorder(ids, this.generateCurves.bind(this));
  }

  forEach(callback: (vertex: VertexEntity, selected: boolean, index: number) => void): void {
    this.m_vertices.forEach((vertex, id, _, index) =>
      callback(vertex, this.selection.has(id), index)
    );
  }

  forEachBezier(callback: (bezier: BezierEntity) => void): void {
    this.m_curves.forEach((curve) => callback(curve));
  }

  reverse(): void {
    CommandHistory.pushMiniBatch();
    this.m_vertices.reverse();
    this.regen();
    CommandHistory.popMiniBatch();

    this.m_vertices.forEach((vertex) => {
      const left = vertex.left;
      const right = vertex.right;

      vertex.left = right;
      vertex.right = left;
    });
  }

  concat(element: ElementEntity): void {
    const mid = vec2.sub(this.transform.center, this.transform.position.value);
    const angle = this.transform.rotation.value;

    const mid1 = vec2.sub(element.transform.center, element.transform.position.value);
    const angle1 = element.transform.rotation.value;

    element.forEach((vertex) => {
      vertex.transform.position.value = vec2.rotate(
        vec2.add(
          vec2.rotate(vertex.transform.position.value, mid1, angle1),
          vec2.sub(element.transform.position.static, this.transform.position.static)
        ),
        mid,
        -angle
      );

      if (vertex.transform.left)
        vertex.transform.left.value = vec2.rotate(
          vertex.transform.left.value,
          [0, 0],
          angle1 - angle
        );
      if (vertex.transform.right)
        vertex.transform.right.value = vec2.rotate(
          vertex.transform.right.value,
          [0, 0],
          angle1 - angle
        );
    });

    CommandHistory.pushMiniBatch();
    element.forEach((vertex) => {
      this.m_vertices.set(vertex.id, vertex);
    });
    this.regen();
    CommandHistory.popMiniBatch();

    element.parent.remove(element.id);

    this.transform.keepCentered(mid, true);
  }

  split(bezier: BezierEntity, position: vec2): VertexEntity | void {
    if (!this.m_curves.has(bezier.id)) return;

    if (this.transform.rotation.value !== 0)
      position = vec2.rotate(position, this.transform.center, -this.transform.rotation.value);

    position = vec2.sub(position, this.transform.position.value);

    const vertex = bezier.split(position);

    this.add(vertex, true, this.m_vertices.indexOf(bezier.start.id) + 1);

    return vertex;
  }

  add(vertex: VertexEntity, regenerate: boolean = true, index?: number): void {
    if (regenerate) CommandHistory.pushMiniBatch();

    this.m_vertices.set(
      vertex.id,
      vertex,
      index
      // regenerate ? this.generateCurves.bind(this) : undefined
    );

    if (regenerate) {
      this.regen();
      CommandHistory.popMiniBatch();
    }
  }

  close(mergeThreshold: number = 1e-4): void {
    // Check if there are at least 2 vertices
    if (this.length < 2) return;

    // Check if the element is a line segment
    if (this.m_curves.size === 1 && this.m_curves.values().next().value.bezierType === 'linear')
      return;

    const first = this.m_vertices.first!;
    const last = this.m_vertices.last!;

    if (
      vec2.sqrDist(first.transform.position.value, last.transform.position.value) <
      Math.pow(mergeThreshold, 2)
    ) {
      if (last.transform.left) first.transform.leftValue = last.transform.left.value;
      this.remove(last, true);
    }

    const mid = this.transform.center;

    if (this.m_vertices.order[0]) {
      CommandHistory.pushMiniBatch();
      this.m_closed.value = true;
      this.regenerate();
      CommandHistory.popMiniBatch();
    }

    this.transform.keepCentered(mid, true);
  }

  isOpenEnd(id: string): boolean {
    if (
      !this.m_closed.value &&
      (this.m_vertices.order[0] === id ||
        this.m_vertices.order[this.m_vertices.order.length - 1] === id)
    )
      return true;
    return false;
  }

  isFirstVertex(id: string): boolean {
    if (this.m_vertices.order[0] === id) return true;
    return false;
  }

  intersects(box: Box): boolean {
    if (this.m_vertices.size < 1) return false;

    const angle = this.transform.rotation.value;

    if (angle === 0) {
      if (!doesBoxIntersectBox(box, this.transform.boundingBox)) return false;

      box = [
        vec2.sub(box[0], this.transform.position.value),
        vec2.sub(box[1], this.transform.position.value)
      ];

      if (
        this.m_vertices.size < 2 &&
        isPointInBox(this.m_vertices.first!.transform.position.value, box, 5)
      )
        return true;

      return Array.from(this.m_curves.values()).some((segment) => segment.intersectsBox(box));
    } else {
      const unrotatedBox = this.transform.unrotatedBoundingBox;
      const mid = vec2.mid(unrotatedBox[0], unrotatedBox[1]);

      if (!doesBoxIntersectRotatedBox(box, unrotatedBox, angle)) return false;

      const position = this.transform.position.value;
      const rotated: vec2[] = [box[0], [box[1][0], box[0][1]], box[1], [box[0][0], box[1][1]]].map(
        (point) => vec2.sub(vec2.rotate(point as vec2, mid, -angle), position)
      );

      rotated.push(rotated[0]);

      const rects: Box[] = [];

      for (let i = 0, n = rotated.length - 1; i < n; ++i) {
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

  regen() {
    CommandHistory.add(new FunctionCallCommand(this.generateCurves.bind(this)));
  }

  remove(vertex: VertexEntity | true, keepClosed = true): void {
    if (this.m_vertices.size < 3) {
      SceneManager.delete(this, true);
      return;
    }

    const box = this.transform.unrotatedBoundingBox;
    const mid = vec2.mid(box[0], box[1]);

    const vertices = vertex === true ? this.selection.entities : [vertex];
    const indices = vertices.map((vertex) => this.m_vertices.indexOf(vertex.id)).sort();

    let fragments: string[][] = [];

    for (let i = 0, n = indices.length; i < n; ++i) {
      let prev = i < 1 ? -1 : indices[i - 1];
      if (prev !== indices[i] - 1) fragments.push(this.m_vertices.slice(prev + 1, indices[i]));
    }

    if (indices[indices.length - 1] < this.m_vertices.size - 1) {
      fragments.push(this.m_vertices.slice(indices[indices.length - 1] + 1));
    }

    if (this.m_closed.value || keepClosed) {
      if (keepClosed) {
        const order: string[] = [];

        fragments.forEach((fragment) => {
          order.push(...fragment);
        });

        const ids = this.m_vertices.order.filter((id) => order.indexOf(id) === -1);

        // TODO: CREATE FUNCTION CALL COMMAND
        CommandHistory.pushMiniBatch();
        ids.forEach((id) => {
          if (id) this.m_vertices.delete(id);
        });

        if (this.m_closed.value && this.m_vertices.size === 2) {
          if (
            !this.m_vertices.first!.right &&
            !this.m_vertices.first!.left &&
            !this.m_vertices.last!.right &&
            !this.m_vertices.last!.left
          ) {
            this.m_closed.value = false;
          }
        }

        this.regen();
        CommandHistory.popMiniBatch();

        // TOCHECK
        // this.selection.restore(vertices as VertexEntity[]);

        return;
      } else {
        if (
          fragments[0][0] === this.m_vertices.order[0] &&
          fragments[fragments.length - 1][fragments[fragments.length - 1].length - 1] ===
            this.m_vertices.order[this.m_vertices.size - 1]
        ) {
          const last = fragments.pop();
          if (last) fragments[0] = last.concat(fragments[0]);
        }
      }
    }

    const elements: Element[] = [];

    for (let i = 0, n = fragments.length; i < n; ++i) {
      fragments[i] = fragments[i].filter((id) => id != undefined);
      if (fragments[i].length > 1) {
        const element = new Element({
          vertices: fragments[i].map((id) => this.m_vertices.get(id)!),
          position: this.transform.position.static,
          rotation: this.transform.rotation.static,
          fill: this.fill?.asObject(),
          stroke: this.stroke?.asObject()
        });
        element.transform.keepCentered(mid);
        SceneManager.add(element);
        elements.push(element);
      }
    }

    SceneManager.delete(this, true);

    CommandHistory.add(
      new ChangeCommand(
        () => {
          elements.forEach((element) => {
            element.forEach((vertex) => (vertex.parent = element));
          });
          this.selection.clear();
        },
        () => {
          this.forEach((vertex) => (vertex.parent = this));
        }
      )
    );

    // TOCHECK
    this.selection.clear();
  }

  getEntityAt(position: vec2, lowerLevel: boolean, threshold: number): Entity | undefined {
    const angle = this.transform.rotation.value;
    const mid = this.transform.center;

    if (angle !== 0) {
      position = vec2.rotate(position, mid, -this.transform.rotation.value);
    }

    if (
      isPointInBox(
        position,
        lowerLevel ? this.transform.largeBoundingBox : this.transform.unrotatedBoundingBox,
        threshold
      )
    ) {
      position = vec2.sub(position, this.transform.position.value);

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

          if (!this.m_closed.value && this.length > 1)
            intersections += this.m_closingCurve.getLineIntersectionPoints(rect).length;

          if (intersections % 2 !== 0) toReturn = this;
        } else {
          let count = 0;

          const curves = Array.from(this.m_curves.values());
          if (!this.m_closed.value && this.length > 1) curves.push(this.m_closingCurve);

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
      const position = this.transform.position.value;
      const mid = vec2.sub(this.transform.center, position);
      const angle = this.transform.rotation.value;

      box = [vec2.sub(box[0], position), vec2.sub(box[1], position)];

      this.m_vertices.forEach((vertex) => {
        vertex.getEntitiesIn(box, entities, true, angle, mid);
      });
    } else if (this.intersects(box)) entities.add(this);
  }

  private onGetDrawableCacheMiss(): Drawable {
    const drawable: Drawable = { operations: [] };

    let first = true;

    this.m_curves.forEach((bezier) => {
      if (first) {
        drawable.operations.push({ type: 'moveTo', data: [bezier.p0] });
        first = false;
      }

      drawable.operations.push(...bezier.getDrawable().operations);
    });

    if (this.m_closed.value) drawable.operations.push({ type: 'closePath' });

    return drawable;
  }

  getDrawable(useWebGL: boolean = false): Drawable {
    Debugger.time('drwb');
    const drawable = this.m_cache.cached('drawable', this.onGetDrawableCacheMiss.bind(this));
    Debugger.timeEnd('drwb');
    return drawable;
  }

  getOutlineDrawable = this.getDrawable;

  render(): void {
    Renderer.element(this);
  }

  asObject(duplicate?: boolean): ElementObject {
    const obj: ElementObject = {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      transform: this.transform.asObject(),
      vertices: this.m_vertices.map((vertex) => vertex.asObject(duplicate) as VertexObject)
    };

    if (this.m_closed.value === true) obj.closed = true;

    if (this.fill) obj.fill = this.fill.asObject(duplicate);
    if (this.stroke) obj.stroke = this.stroke.asObject(duplicate);

    return obj;
  }

  toJSON(): ElementObject {
    return this.asObject(false);
  }
}

export default Element;
