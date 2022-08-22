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

  public parent: Vertex;

  private readonly m_type: HandleOptions['type'];

  private m_position: vec2;
  private m_transform: SimpleTransform;

  constructor({ position, type, parent }: HandleOptions) {
    this.id = nanoid();
    this.parent = parent as any as Vertex;
    this.m_position = position;
    this.m_type = type;
    this.m_transform = new SimpleTransform();
  }

  public get visible() {
    return true;
  }

  public get position() {
    return vec2.add(this.m_position, this.m_transform.vec2);
  }

  public set position(position: vec2) {
    this.m_position = vec2.clone(position);
  }

  public get staticPosition(): vec2 {
    return this.m_position;
  }

  public get handleType() {
    return this.m_type;
  }

  public move(delta: vec2) {
    HistoryManager.record({
      fn: () => {
        vec2.add(this.m_position, delta, true);
      },
      undo: () => {
        vec2.sub(this.m_position, delta, true);
      }
    });
  }

  public moveTo(position: vec2) {
    const backup = vec2.clone(this.m_position);

    HistoryManager.record({
      fn: () => {
        this.position = position;
      },
      undo: () => {
        this.m_position = backup;
      }
    });
  }

  public translate(delta: vec2, lockMirror = false) {
    this.m_transform.translate(delta);

    if (!lockMirror && this.m_type === 'bezier' && !InputManager.keys.alt)
      this.parent.mirrorTranslation(this.id);

    this.parent.parent.recalculate();
  }

  public translateTo(position: vec2, lockMirror = false) {
  this.m_transform.translate(vec2.sub(position, this.position));

    if (!lockMirror && this.m_type === 'bezier' && !InputManager.keys.alt)
      this.parent.mirrorTranslation(this.id);

    this.parent.parent.recalculate();
  }

  public applyTransform(lockMirror = false) {
    const backup = vec2.clone(this.m_position);
    const transformed = vec2.add(this.m_position, this.m_transform.vec2);

    if (!vec2.equals(transformed, backup)) {
      HistoryManager.record({
        fn: () => {
          this.m_position = transformed;
          this.parent.parent.recalculate();
        },
        undo: () => {
          this.m_position = backup;
          this.parent.parent.recalculate();
        }
      });

      if (!lockMirror && this.m_type === 'bezier') this.parent.applyMirroredTranslation(this.id);

      this.m_transform.clear();
    }
  }

  public clearTransform(lockMirror = false) {
    this.m_transform.clear();

    if (!lockMirror && this.m_type === 'bezier') this.parent.clearMirroredTranslation(this.id);

    this.parent.parent.recalculate();
  }

  public getEntityAt(position: vec2, lowerLevel = true, threshold: number = 0) {
    if (isPointInCircle(position, this.m_position, threshold)) return this;
    return undefined;
  }

  public getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean | undefined): void {}

  public delete() {}

  public render() {}

  public toJSON() {
    return {} as HandleObject;
  }
}

export default Handle;
