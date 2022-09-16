import Cache from '@utils/cache';
import { doesBoxIntersectsBox, isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../history';
import { Renderer } from '../renderer';
import SceneManager from '../scene';
import { ElementSelectionManager } from '../selection';
import Bezier from './bezier';
import Transform from './components/transform';
import Layer from './layer';
import Vertex from './vertex';
import Fill from './components/fill';
import Stroke from './components/stroke';

class Element implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'element';
  public readonly selection = new ElementSelectionManager(this);

  public parent: Layer;

  private m_order: string[] = [];
  private m_vertices: Map<string, Vertex> = new Map();
  private m_curves: Map<string, Bezier> = new Map();
  private m_closed: boolean;
  private m_position: vec2;
  private m_transform: Transform;
  private m_stroke: string | null = null;
  private m_fill: string | null = null;

  private m_cache: Cache = new Cache();
  private cached = this.m_cache.cached.bind(this.m_cache);

  constructor({ id = nanoid(), position, vertices, closed = false, stroke, fill }: ElementOptions) {
    this.id = id;
    this.m_closed = closed;
    this.m_position = position;
    this.m_transform = new Transform();

    if (vertices) this.vertices = vertices as any as Vertex[];

    if (stroke) {
      if (typeof stroke === 'string') {
        if (SceneManager.assets.has(stroke, 'stroke')) {
          const s = SceneManager.assets.get(stroke, 'stroke');
          s!.addParent(this);
          this.m_stroke = stroke;
        } else {
          const s = new Stroke({});
          SceneManager.assets.set(s);
          s.addParent(this);
          this.m_stroke = s.id;
        }
      } else {
        this.m_stroke = stroke.id;
        (stroke as any).addParent(this);
        if (!SceneManager.assets.has(stroke.id, 'stroke')) SceneManager.assets.set(stroke as any);
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
          SceneManager.assets.set(f);
          f.addParent(this);
          this.m_fill = f.id;
        }
      } else {
        this.m_fill = fill.id;
        (fill as any).addParent(this);
        if (!SceneManager.assets.has(fill.id, 'stroke')) SceneManager.assets.set(fill as any);
      }
    }
  }

  public get visible() {
    const box = this.boundingBox;
    const position = SceneManager.viewport.position;
    const canvasSize = vec2.sub(
      vec2.div(Renderer.size, SceneManager.viewport.zoom),
      SceneManager.viewport.position
    );

    return (
      box[1][0] >= -position[0] &&
      box[0][0] <= canvasSize[0] &&
      box[1][1] >= -position[1] &&
      box[0][1] <= canvasSize[1]
    );
  }

  public get position() {
    return this.m_position;
  }

  public set position(position: vec2) {
    this.m_position = vec2.clone(position);
  }

  public get boundingBox() {
    return this.cached<Box>('boundingBox', () => {
      let min: vec2 = [Infinity, Infinity];
      let max: vec2 = [-Infinity, -Infinity];

      this.m_curves.forEach((bezier) => {
        const box = bezier.boundingBox;

        min = vec2.min(min, box[0]);
        max = vec2.max(max, box[1]);
      });

      return [vec2.add(min, this.m_position), vec2.add(max, this.m_position)] as Box;
    });
  }

  public get largeBoundingBox() {
    return this.cached<Box>('largeBoundingBox', () => {
      const box = this.boundingBox;

      let min: vec2 = vec2.sub(box[0], this.m_position);
      let max: vec2 = vec2.sub(box[1], this.m_position);

      this.m_vertices.forEach((vertex) => {
        const vertexBox = vertex.boundingBox;

        min = vec2.min(min, vertexBox[0]);
        max = vec2.max(max, vertexBox[1]);
      });

      return [vec2.add(min, this.m_position), vec2.add(max, this.m_position)] as Box;
    });
  }

  public get vertexCount() {
    return this.m_order.length;
  }

  public get lastVertex() {
    return this.m_vertices.get(this.m_order[this.m_order.length - 1])!;
  }

  public get transform() {
    return this.m_transform.mat4;
  }

  public set vertices(vertices: Vertex[]) {
    this.m_order.length = 0;

    this.m_vertices.clear();

    vertices.forEach((vertex) => {
      this.push(vertex, false);
    });

    this.recalculate(false);
    this.generateCurves();
  }

  public move(delta: vec2) {
    HistoryManager.record({
      fn: () => {
        vec2.add(this.m_position, delta, true);
        this.recalculate(false);
      },
      undo: () => {
        vec2.sub(this.m_position, delta, true);
        this.recalculate(false);
      }
    });
  }

  public moveTo(position: vec2) {
    const backup = vec2.clone(this.m_position);

    HistoryManager.record({
      fn: () => {
        this.position = position;
        this.recalculate(false);
      },
      undo: () => {
        this.m_position = backup;
        this.recalculate(false);
      }
    });
  }

  public translate(delta: vec2) {
    this.m_transform.translate(delta);
  }

  public applyTransform() {
    const backup = vec2.clone(this.m_position);
    const transformed = vec2.transformMat4(this.m_position, this.m_transform.mat4);

    if (!vec2.equals(transformed, backup)) {
      HistoryManager.record({
        fn: () => {
          this.m_position = transformed;
          this.recalculate(false);
        },
        undo: () => {
          this.m_position = backup;
          this.recalculate(false);
        }
      });

      this.m_transform.clear();
    }
  }

  public clearTransform() {
    this.m_transform.clear();
  }

  public recalculate(propagate = true) {
    this.m_cache.clear();
    if (propagate) this.m_curves.forEach((bezier) => bezier.recalculate());
  }

  public forEach(callback: (vertex: Vertex, selected?: boolean) => void) {
    this.m_vertices.forEach((vertex) => callback(vertex, this.selection.has(vertex.id)));
  }

  public intersects(box: Box): boolean {
    if (!doesBoxIntersectsBox(box, this.boundingBox)) return false;

    box = [vec2.sub(box[0], this.m_position), vec2.sub(box[1], this.m_position)];

    if (
      this.vertexCount < 2 &&
      isPointInBox(Array.from(this.vertices.values())[0].position, box, 5)
    )
      return true;

    return Array.from(this.m_curves.values()).some((segment) => segment.intersectsBox(box));
  }

  public generateCurves(ids: string[] = this.m_order) {
    const curves = new Map<string, Bezier>();
    const vertices = new Map<string, Vertex>();

    let last: Vertex | null = null;

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

  public reverseCurves() {
    // TODO: fix reverseCurves causing history problems
    this.m_vertices.forEach((vertex) => {
      const left = vertex.left;
      const right = vertex.right;

      vertex.setLeft(right);
      vertex.setRight(left);
    });

    HistoryManager.record({
      fn: () => {
        this.generateCurves(this.m_order.reverse());
      },
      undo: () => {
        this.generateCurves(this.m_order.reverse());
      }
    });
  }

  public concat(element: Element) {
    const backup = [...this.m_order];

    HistoryManager.record({
      fn: () => {
        element.forEach((vertex) => {
          vertex.translate(vec2.sub(element.position, this.m_position));
          this.push(vertex, false);
        });

        this.generateCurves();
        element.parent.remove(element.id, true);
      },
      undo: () => {
        element.forEach((vertex) => {
          vertex.translate(vec2.sub(this.m_position, element.position));
          this.splice(vertex.id, false);

          vertex.parent = element;
        });

        element.parent.add(element, true);
        this.generateCurves(backup);
      }
    });
  }

  public splitCurve(bezier: Bezier, position: vec2) {
    if (!this.m_curves.has(bezier.id)) return;

    position = vec2.sub(position, this.m_position);

    const vertex = bezier.split(position);

    this.pushVertex(vertex, true, this.m_order.indexOf(bezier.getStart().id) + 1);

    return vertex;
  }

  public isOpenEnd(id: string): boolean {
    if (!this.m_closed && (this.m_order[0] === id || this.m_order[this.m_order.length - 1] === id))
      return true;
    return false;
  }

  public isFirstVertex(id: string): boolean {
    if (this.m_order[0] === id) return true;
    return false;
  }

  public close(mergeThreshold: number = 1e-4) {
    // Check if there are at least 2 vertices
    if (this.vertexCount < 2) return;

    // Check if the element is a line segment
    if (this.m_curves.size === 1 && this.m_curves.values().next().value.bezierType === 'linear')
      return;

    const first = this.m_vertices.get(this.m_order[0])!;
    const last = this.m_vertices.get(this.m_order[this.m_order.length - 1])!;

    if (vec2.sqrDist(first.position, last.position) < Math.pow(mergeThreshold, 2)) {
      if (last.left) first.setLeft(last.left.position);
      this.delete(last, true);
    }

    if (this.m_order[0])
      HistoryManager.record({
        fn: () => {
          this.m_closed = true;
          this.generateCurves();
        },
        undo: () => {
          this.m_closed = false;
          this.generateCurves();
        }
      });
  }

  private push(vertex: Vertex, generateCurves = true, index: number = this.m_order.length) {
    this.m_vertices.set(vertex.id, vertex);
    this.m_order.splice(index, 0, vertex.id);

    vertex.parent = this;

    this.recalculate(false);
    if (generateCurves) this.generateCurves();
  }

  private splice(id: string, generateCurves = true, index: number = this.m_order.indexOf(id)) {
    this.m_vertices.delete(id);
    this.m_order.splice(index, 1);

    this.recalculate(false);
    if (generateCurves) this.generateCurves();
  }

  public pushVertex(vertex: Vertex, generateCurves = true, index?: number) {
    HistoryManager.record({
      fn: () => {
        this.push(vertex, generateCurves, index);
      },
      undo: () => {
        this.splice(vertex.id, generateCurves);
      }
    });
  }

  public getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    if (isPointInBox(position, lowerLevel ? this.largeBoundingBox : this.boundingBox, threshold)) {
      position = vec2.sub(position, this.m_position);

      let toReturn: Entity | undefined = undefined;

      this.m_vertices.forEach((vertex) => {
        if (!toReturn) {
          const result = vertex.getEntityAt(position, lowerLevel, threshold);

          if (result) toReturn = result;
        }
      });

      if (!toReturn) {
        this.m_curves.forEach((bezier) => {
          if (!toReturn) {
            const result = bezier.getEntityAt(position, lowerLevel, threshold);

            if (result) toReturn = result;
          }
        });
      }

      return toReturn;
    }

    return undefined;
  }

  public getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel = false) {
    if (lowerLevel) {
      box = [vec2.sub(box[0], this.m_position), vec2.sub(box[1], this.m_position)];

      this.m_vertices.forEach((vertex) => {
        vertex.getEntitiesIn(box, entities);
      });
    } else if (this.intersects(box)) {
      entities.add(this);
    }
  }

  // TODO: fix deleting multiple vertices in the middle of an element
  public delete(vertex: Vertex, keepClosed = false) {
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

          this.recalculate(false);
          this.generateCurves();
        },
        undo: () => {
          this.m_vertices.set(vertex.id, vertex);

          this.recalculate(false);
          this.generateCurves(backup);
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
            this.generateCurves(after.concat(before));
          } else {
            if (before.length === 1) [before, after] = [after, before];

            const object = this.toJSON(true);

            object.vertices = after.map((id) => this.m_vertices.get(id)!.toJSON());
            newElement = SceneManager.fromObject(object) as Element;

            if (newElement.vertexCount > 1) (this.parent as Layer).add(newElement, true);

            this.generateCurves(before);
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

            if (newElement.vertexCount > 1) SceneManager.remove(newElement, true);
          }

          this.recalculate(false);
          this.generateCurves(backup);
        }
      });
    }
  }

  public deleteSelf() {
    const stroke = this.m_stroke ? SceneManager.assets.get(this.m_stroke, 'stroke') : null;
    const fill = this.m_fill ? SceneManager.assets.get(this.m_fill, 'fill') : null;

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
  }

  public render() {
    Renderer.element(this);
  }

  public getDrawable(useWebGL = false): Drawable {
    return this.cached<Drawable>(`getDrawable${useWebGL ? '-gl' : ''}`, () => {
      if (useWebGL) {
        return { operations: [{ type: 'geometry' }] };
      } else {
        const drawable: Drawable = { operations: [{ type: 'begin' }] };

        let first = true;

        this.m_curves.forEach((bezier) => {
          if (first) {
            drawable.operations.push({ type: 'move', data: [bezier.start] });
            first = false;
          }

          drawable.operations.push(bezier.getDrawOp());
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

  public getOutlineDrawable(useWebGL = false): Drawable {
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

  public toJSON(duplicate = false) {
    const obj: ElementObject = {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: vec2.clone(this.m_position),
      vertices: this.m_order.map((id) => this.m_vertices.get(id)!.toJSON(duplicate))
    };

    if (this.m_closed) obj.closed = true;

    if (this.m_stroke) obj.stroke = this.m_stroke;
    if (this.m_fill) obj.fill = this.m_fill;

    return obj;
  }
}

export default Element;
