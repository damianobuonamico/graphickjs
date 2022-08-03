import { vec2 } from '@/math';
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

  render() {}

  public toJSON() {
    return {
      id: this.id,
      type: this.type,
      position: this.position,
      left: this.m_left ? this.m_left.position : undefined,
      right: this.m_right ? this.m_right.position : undefined
    };
  }
}

export default Vertex;
