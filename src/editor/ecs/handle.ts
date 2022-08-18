import { isPointInCircle, vec2 } from '@math';
import { nanoid } from 'nanoid';
import InputManager from '../input';
import Element from './element';
import Vertex from './vertex';

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

  public get handleType() {
    return this.m_type;
  }

  public get position() {
    return vec2.clone(this.m_position);
  }

  public set position(position: vec2) {
    this.m_position = vec2.clone(position);
  }

  public get visible() {
    return true;
  }

  public translate(delta: vec2, lockMirror = false) {
    vec2.add(this.m_position, delta, true);
    if (!lockMirror && this.m_type === 'bezier' && !InputManager.keys.alt)
      (this.parent as Vertex).mirrorTranslation(this.id);
    (this.parent.parent as Element).recalculate();
  }

  render() {}
  delete() {}
  applyTransform() {}
  public clearTransform() {}
  public toJSON() {
    return {} as HandleObject;
  }

  public getEntityAt(position: vec2, lowerLevel = true, threshold: number = 0) {
    if (isPointInCircle(position, this.m_position, threshold)) return this;
    return undefined;
  }
  public getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {}
}

export default Handle;
