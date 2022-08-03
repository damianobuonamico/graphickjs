import { vec2 } from '@/math';
import { nanoid } from 'nanoid';
import { Renderer } from '../renderer';
import Vertex from './vertex';

class Element implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'element';
  public parent: Entity;

  private m_vertices: Map<string, Vertex> = new Map();
  private m_position: vec2;

  constructor({ id = nanoid(), position, vertices }: ElementOptions) {
    this.id = id;
    this.m_position = position;
    if (vertices) this.vertices = vertices as Vertex[];
  }

  public set vertices(vertices: Vertex[]) {
    this.m_vertices.clear();
    vertices.forEach((vertex) => {
      this.m_vertices.set(vertex.id, vertex);
    });
  }

  public render() {
    this.m_vertices.forEach((vertex) => {
      Renderer.rect({
        pos: vec2.add(this.m_position, vertex.position),
        size: [10, 10],
        color: [0.0, 0.0, 0.0, 1.0],
        centered: true
      });
    });
  }

  public toJSON() {
    return {
      id: this.id,
      type: this.type,
      position: this.m_position,
      vertices: Array.from(this.m_vertices.values()).map((vertex) => vertex.toJSON())
    };
  }
}

export default Element;
