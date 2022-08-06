import { isPointInBox, vec2 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../history';
import { Renderer } from '../renderer';
import Transform from './components/transform';
import Vertex from './vertex';

class Element implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'element';
  public parent: Entity;

  private m_vertices: Map<string, Vertex> = new Map();
  private m_position: vec2;
  private m_transform: Transform;

  constructor({ id = nanoid(), position, vertices }: ElementOptions) {
    this.id = id;
    this.m_position = position;
    this.m_transform = new Transform();
    if (vertices) this.vertices = vertices as Vertex[];
  }

  public get boundingBox() {
    let min_: vec2 = [Infinity, Infinity];
    let max_: vec2 = [-Infinity, -Infinity];
    this.m_vertices.forEach((vertex) => {
      min_ = vec2.min(min_, vertex.position);
      max_ = vec2.max(max_, vertex.position);
    });
    return [vec2.add(min_, this.m_position), vec2.add(max_, this.m_position)] as Box;
  }

  public set vertices(vertices: Vertex[]) {
    this.m_vertices.clear();
    vertices.forEach((vertex) => {
      this.m_vertices.set(vertex.id, vertex);
      vertex.parent = this;
    });
  }

  public translate(delta: vec2) {
    this.m_transform.translate(delta);
  }

  public render() {
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
}

export default Element;
