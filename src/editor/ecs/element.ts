import { isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../history';
import { Renderer } from '../renderer';
import Bezier from './bezier';
import Transform from './components/transform';
import Vertex from './vertex';

class Element implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'element';
  public parent: Entity;

  private m_vertices: Map<string, Vertex> = new Map();
  private m_curves: Map<string, Bezier> = new Map();
  private m_closed: boolean;
  private m_position: vec2;
  private m_transform: Transform;

  constructor({ id = nanoid(), position, vertices, closed = false }: ElementOptions) {
    this.id = id;
    this.m_closed = closed;
    this.m_position = position;
    this.m_transform = new Transform();
    if (vertices) this.vertices = vertices as Vertex[];
  }

  public get position() {
    return this.m_position;
  }

  public get boundingBox() {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];
    this.m_vertices.forEach((vertex) => {
      min = vec2.min(min, vertex.position);
      max = vec2.max(max, vertex.position);
    });
    return [vec2.add(min, this.m_position), vec2.add(max, this.m_position)] as Box;
  }

  public set vertices(vertices: Vertex[]) {
    this.m_vertices.clear();
    vertices.forEach((vertex) => {
      this.m_vertices.set(vertex.id, vertex);
      vertex.parent = this;
    });
    this.generateCurves();
  }

  public get transform() {
    return this.m_transform.mat4;
  }

  public translate(delta: vec2) {
    this.m_transform.translate(delta);
  }

  public render() {
    Renderer.element(this);
    this.m_vertices.forEach((vertex) => {
      Renderer.rect({
        pos: vec2.add(this.m_position, vertex.position),
        size: [10, 10],
        color: [0.0, 0.0, 0.0, 1.0],
        centered: true,
        transform: this.m_transform.mat4
      });
    });
  }

  public toJSON(duplicate = false) {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      closed: this.m_closed,
      position: vec2.clone(this.m_position),
      vertices: Array.from(this.m_vertices.values()).map((vertex) => vertex.toJSON(duplicate))
    };
  }

  public getEntityAt(position: vec2, threshold: number = 0) {
    if (isPointInBox(position, this.boundingBox, threshold)) {
      position = vec2.sub(position, this.m_position);
      let toReturn = this;
      this.m_vertices.forEach((vertex) => {
        const result = vertex.getEntityAt(position, threshold);
        if (result) toReturn = result;
      });
      return toReturn;
    }
    return undefined;
  }

  public delete(entity: Entity) {
    if (!this.m_vertices.has(entity.id)) return;
    HistoryManager.record({
      fn: () => {
        this.m_vertices.delete(entity.id);
      },
      undo: () => {
        this.m_vertices.set(entity.id, entity as unknown as Vertex);
        entity.parent = this;
      }
    });
  }

  public applyTransform() {
    const backup = vec2.clone(this.m_position);
    const transformed = vec2.transformMat4(this.m_position, this.m_transform.mat4);
    if (!vec2.equals(transformed, backup)) {
      HistoryManager.record({
        fn: () => {
          this.m_position = transformed;
        },
        undo: () => {
          this.m_position = backup;
        }
      });
      this.m_transform.clear();
    }
  }

  private generateCurves(ids: string[] = Array.from(this.m_vertices.keys())) {
    const vertices = new Map<string, Vertex>();
    const curves = new Map<string, Bezier>();

    let last: Vertex | null = null;
    ids.forEach((id) => {
      const vertex = this.m_vertices.get(id)!;
      vertices.set(id, vertex);
      if (last) {
        const bezier = new Bezier({ start: last, end: vertex });
        curves.set(bezier.id, bezier);
      }
      last = vertex;
    });

    if (this.m_closed) {
      const bezier = new Bezier({
        start: vertices.get(ids[ids.length - 1])!,
        end: vertices.get(ids[0])!
      });
      curves.set(bezier.id, bezier);
    }

    this.m_vertices = vertices;
    this.m_curves = curves;
  }

  public getDrawable(useWebGL = false): Drawable {
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
      drawable.operations.push({ type: 'close' });
      drawable.operations.push({ type: 'stroke' });
      return drawable;
    }
  }
}

export default Element;
