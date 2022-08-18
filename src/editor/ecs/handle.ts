import { isPointInCircle, vec2 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../history';
import InputManager from '../input';
import { SimpleTransform } from './components/transform';
import Element from './element';
import Vertex from './vertex';

class Handle implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'handle';
  public parent: Entity;

  private readonly m_type: HandleOptions['type'];

  private m_position: vec2;
  private m_transform: SimpleTransform;

  constructor({ position, type, parent }: HandleOptions) {
    this.id = nanoid();
    this.parent = parent;
    this.m_position = position;
    this.m_type = type;
    this.m_transform = new SimpleTransform();
  }

  public get handleType() {
    return this.m_type;
  }

  public get position() {
    return vec2.add(this.m_position, this.m_transform.vec2);
  }

  public set position(position: vec2) {
    this.m_position = vec2.clone(position);
  }

  public get visible() {
    return true;
  }

  public translate(delta: vec2, lockMirror = false) {
    this.m_transform.translate(delta);
    if (!lockMirror && this.m_type === 'bezier' && !InputManager.keys.alt)
      (this.parent as Vertex).mirrorTranslation(this.id);
    (this.parent.parent as Element).recalculate();
  }

  render() {}
  delete() {}
  public applyTransform(lockMirror = false) {
    const backup = vec2.clone(this.m_position);
    const transformed = vec2.add(this.m_position, this.m_transform.vec2);
    if (!vec2.equals(transformed, backup)) {
      HistoryManager.record({
        fn: () => {
          this.m_position = transformed;
          (this.parent.parent as Element).recalculate();
        },
        undo: () => {
          this.m_position = backup;
          (this.parent.parent as Element).recalculate();
        }
      });
      if (!lockMirror && this.m_type === 'bezier')
        (this.parent as Vertex).applyMirroredTranslation(this.id);
      this.m_transform.clear();
    }
  }
  public clearTransform(lockMirror = false) {
    this.m_transform.clear();
    if (!lockMirror && this.m_type === 'bezier')
      (this.parent as Vertex).clearMirroredTranslation(this.id);
    (this.parent.parent as Element).recalculate();
  }
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
