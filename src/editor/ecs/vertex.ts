import { vec2 } from '@math';
import { nanoid } from 'nanoid';
import Handle from './handle';

class Vertex implements VertexEntity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'vertex';
  public parent: Entity;

  private m_position: Handle;
  private m_left?: Handle;
  private m_right?: Handle;

  constructor({ id = nanoid(), position, left, right }: VertexOptions) {
    this.id = id;
    this.m_position = new Handle({ position, type: 'vertex', parent: this });
    if (left) this.m_left = new Handle({ position: left, type: 'bezier', parent: this });
    if (right) this.m_right = new Handle({ position: right, type: 'bezier', parent: this });
  }

  public get position() {
    return this.m_position.position;
  }

  public get left() {
    return this.m_left;
  }

  public get right() {
    return this.m_right;
  }

  public translate(delta: vec2) {
    this.m_position.translate(delta);
  }

  delete() {}

  render() {}

  public toJSON(duplicate = false) {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: vec2.clone(this.position),
      left: this.m_left ? vec2.clone(this.m_left.position) : undefined,
      right: this.m_right ? vec2.clone(this.m_right.position) : undefined
    };
  }

  public getEntityAt(position: vec2, threshold: number = 0) {
    let toReturn = undefined;
    [this.m_position, this.m_left, this.m_right].forEach((handle) => {
      if (handle) {
        const result = handle.getEntityAt(position, threshold);
        if (result) toReturn = result;
      }
    });
    return toReturn;
  }

  public applyTransform() {}
}

export default Vertex;
