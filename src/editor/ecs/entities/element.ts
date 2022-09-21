import Cache from '@utils/cache';
import { doesBoxIntersectBox, isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../../history';
import { Renderer } from '../../renderer';
import SceneManager from '../../scene';
import { ElementSelectionManager } from '../../selection';
import Bezier from './bezier';
import { Transform } from '../components/transform';
import Layer from './layer';
import Vertex from './vertex';
import Fill from '../components/fill';
import Stroke from '../components/stroke';

class Element implements ElementEntity {
  readonly id: string;
  readonly type: EntityType = 'element';
  readonly selection = new ElementSelectionManager(this);

  parent: Layer;
  transform: TransformComponent;

  private m_order: string[] = [];
  private m_vertices: Map<string, VertexEntity> = new Map();
  private m_curves: Map<string, BezierEntity> = new Map();
  private m_closed: boolean;
  private m_stroke: string | null = null;
  private m_fill: string | null = null;
  private m_recordHistory: boolean;

  private m_cache: Cache = new Cache();
  private cached = this.m_cache.cached.bind(this.m_cache);

  constructor({
    id = nanoid(),
    vertices,
    position,
    closed = false,
    stroke,
    fill,
    recordHistory = true
  }: ElementOptions) {
    this.id = id;
    this.m_closed = closed;
    this.transform = new Transform(position, undefined, undefined, () => this.recalculate());
    this.m_recordHistory = recordHistory;

    if (vertices) this.vertices = vertices;

    // TODO: refactor
    if (stroke) {
      if (typeof stroke === 'string') {
        if (SceneManager.assets.has(stroke, 'stroke')) {
          const s = SceneManager.assets.get(stroke, 'stroke');
          s!.addParent(this);
          this.m_stroke = stroke;
        } else {
          const s = new Stroke({});
          SceneManager.assets.set(s, !this.m_recordHistory);
          s.addParent(this);
          this.m_stroke = s.id;
        }
      } else {
        this.m_stroke = stroke.id;
        (stroke as any).addParent(this);
        if (!SceneManager.assets.has(stroke.id, 'stroke'))
          SceneManager.assets.set(stroke as any, !this.m_recordHistory);
      }
    }

    if (fill) {
      if (typeof fill === 'string') {
        if (SceneManager.assets.has(fill, 'fill')) {
          const f = SceneManager.assets.get(fill, 'fill')!;
          f.addParent(this);
          this.m_fill = fill;
        } else {
          const f = new Fill({});
          SceneManager.assets.set(f, !this.m_recordHistory);
          f.addParent(this);
          this.m_fill = f.id;
        }
      } else {
        this.m_fill = fill.id;
        (fill as any).addParent(this);
        if (!SceneManager.assets.has(fill.id, 'stroke'))
          SceneManager.assets.set(fill as any, !this.m_recordHistory);
      }
    }
  }

  get boundingBox(): Box {
    return this.cached<Box>('boundingBox', () => {
      let min: vec2 = [Infinity, Infinity];
      let max: vec2 = [-Infinity, -Infinity];

      this.m_curves.forEach((bezier) => {
        const box = bezier.boundingBox;

        min = vec2.min(min, box[0]);
        max = vec2.max(max, box[1]);
      });

      return [vec2.add(min, this.transform.position), vec2.add(max, this.transform.position)];
    });
  }

  get largeBoundingBox(): Box {
    return this.cached<Box>('largeBoundingBox', () => {
      const box = this.boundingBox;
      const position = this.transform.position;

      let min: vec2 = vec2.sub(box[0], position);
      let max: vec2 = vec2.sub(box[1], position);

      this.m_vertices.forEach((vertex) => {
        const vertexBox = vertex.boundingBox;

        min = vec2.min(min, vertexBox[0]);
        max = vec2.max(max, vertexBox[1]);
      });

      return [vec2.add(min, position), vec2.add(max, position)];
    });
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

  private pushVertex(
    vertex: VertexEntity,
    regenerate = true,
    index: number = this.m_order.length
  ): void {
    this.m_vertices.set(vertex.id, vertex);
    this.m_order.splice(index, 0, vertex.id);

    vertex.parent = this;

    if (regenerate) this.regenerate();
    else this.recalculate(false);
  }

  private spliceVertex(
    id: string,
    regenerate = true,
    index: number = this.m_order.indexOf(id)
  ): void {
    this.m_vertices.delete(id);
    this.m_order.splice(index, 1);

    if (regenerate) this.regenerate();
    else this.recalculate(false);
  }

  recalculate(propagate: boolean = true): void {
    this.m_cache.clear();
    if (propagate) this.m_curves.forEach((bezier) => bezier.recalculate());
  }

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

    this.recalculate(false);
  }

  forEach(callback: (vertex: VertexEntity, selected?: boolean | undefined) => void): void {
    this.m_vertices.forEach((vertex) => callback(vertex, this.selection.has(vertex.id)));
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

    element.forEach((vertex) => {
      vertex.transform.move(
        vec2.sub(element.transform.staticPosition, this.transform.staticPosition)
      );
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
  }

  split(bezier: BezierEntity, position: vec2): VertexEntity | void {
    if (!this.m_curves.has(bezier.id)) return;

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
    if (!doesBoxIntersectBox(box, this.boundingBox)) return false;

    box = [vec2.sub(box[0], this.transform.position), vec2.sub(box[1], this.transform.position)];

    if (
      this.length < 2 &&
      isPointInBox(Array.from(this.vertices.values())[0].transform.position, box, 5)
    )
      return true;

    return Array.from(this.m_curves.values()).some((segment) => segment.intersectsBox(box));
  }

  delete(vertex: VertexEntity, keepClosed = false): void {
    if (!this.m_vertices.has(vertex.id)) return;

    const index = this.m_order.indexOf(vertex.id);
    const backup = [...this.m_order];
    const wasClosed = this.m_closed;

    if (this.m_order.length < 3) {
      this.selection.all();
      SceneManager.delete(this);

      return;
    }

    if (keepClosed || this.isOpenEnd(vertex.id)) {
      HistoryManager.record({
        fn: () => {
          this.m_order.splice(index, 1);
          this.m_vertices.delete(vertex.id);

          this.regenerate();
        },
        undo: () => {
          this.m_vertices.set(vertex.id, vertex);

          this.regenerate(backup);
        }
      });
    } else {
      if (!this.m_closed && this.m_order.length < 4) {
        this.selection.all();
        SceneManager.delete(this);

        return;
      }

      let newElement: Element | undefined = undefined;

      HistoryManager.record({
        fn: () => {
          this.m_closed = false;

          let before = this.m_order.slice(0, index);
          let after = this.m_order.slice(index + 1, this.m_order.length);

          if (wasClosed) {
            this.regenerate(after.concat(before));
          } else {
            if (before.length === 1) [before, after] = [after, before];

            const object = this.asObject(true);

            object.vertices = after.map(
              (id) => this.m_vertices.get(id)!.asObject() as VertexObject
            );
            newElement = SceneManager.fromObject(object) as Element;

            if (newElement.length > 1) (this.parent as Layer).add(newElement, true);

            this.regenerate(before);
          }

          this.recalculate(false);
        },
        undo: () => {
          this.m_closed = wasClosed;

          this.m_vertices.set(vertex.id, vertex);

          if (!wasClosed && newElement) {
            newElement.forEach((v) => {
              this.m_vertices.set(v.id, v);
              v.parent = this;
            });

            if (newElement.length > 1) SceneManager.remove(newElement, true);
          }

          this.regenerate(backup);
        }
      });
    }
  }

  destroy(): void {
    const stroke = this.m_stroke ? SceneManager.assets.get(this.m_stroke, 'stroke') : null;
    const fill = this.m_fill ? SceneManager.assets.get(this.m_fill, 'fill') : null;

    if (this.m_recordHistory) {
      HistoryManager.record({
        fn: () => {
          if (stroke) stroke.deleteParent(this);
          if (fill) fill.deleteParent(this);
        },
        undo: () => {
          if (stroke) stroke.addParent(this);
          if (fill) fill.addParent(this);
        }
      });
    } else {
      if (stroke) stroke.deleteParent(this);
      if (fill) fill.deleteParent(this);
    }
  }

  getEntityAt(position: vec2, lowerLevel: boolean, threshold: number): Entity | undefined {
    if (isPointInBox(position, lowerLevel ? this.largeBoundingBox : this.boundingBox, threshold)) {
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

      return toReturn;
    }

    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean): void {
    if (lowerLevel) {
      box = [vec2.sub(box[0], this.transform.position), vec2.sub(box[1], this.transform.position)];

      this.m_vertices.forEach((vertex) => {
        vertex.getEntitiesIn(box, entities);
      });
    } else if (this.intersects(box)) entities.add(this);
  }

  getDrawable(useWebGL: boolean = false): Drawable {
    return this.cached<Drawable>(`getDrawable${useWebGL ? '-gl' : ''}`, () => {
      if (useWebGL) {
        return { operations: [{ type: 'geometry' }] };
      } else {
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

        if (this.m_fill) {
          drawable.operations.push({
            type: 'fillcolor',
            data: SceneManager.assets.get(this.m_fill, 'fill').color.vec4
          });

          drawable.operations.push({ type: 'fill' });
        }

        if (this.m_stroke) {
          drawable.operations.push({
            type: 'strokecolor',
            data: SceneManager.assets.get(this.m_stroke, 'stroke').color.vec4
          });

          drawable.operations.push({ type: 'stroke' });
        }

        return drawable;
      }
    });
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
      position: this.transform.position,
      vertices: this.m_order.map(
        (id) => this.m_vertices.get(id)!.asObject(duplicate) as VertexObject
      )
    };

    if (this.m_closed) obj.closed = true;

    if (this.m_stroke) obj.stroke = this.m_stroke;
    if (this.m_fill) obj.fill = this.m_fill;

    return obj;
  }

  toJSON(): ElementObject {
    return this.asObject(false);
  }
}

export default Element;
