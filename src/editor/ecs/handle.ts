import { vec2 } from '@/math';
import { nanoid } from 'nanoid';

class Handle implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'handle';
  public parent: Entity;

  private readonly m_type: HandleOptions['type'];

  private m_position: vec2;

  constructor({ position, type, parent }: HandleOptions) {
    this.id = nanoid();
    this.parent = parent;
    this.m_position = position;
    this.m_type = type;
  }

  public get position() {
    return vec2.clone(this.m_position);
  }

  render() {}
}

export default Handle;
