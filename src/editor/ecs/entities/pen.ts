import { Renderer } from '@/editor/renderer';
import SceneManager from '@/editor/scene';
import { nanoid } from 'nanoid';
import { UntrackedTransform } from '../components/transform';

class Pen implements PenEntity {
  readonly id: string;
  readonly type = 'pen';
  readonly selectable = false;
  readonly transform: UntrackedTransform;

  parent: Entity;

  private m_p0: vec2 | undefined = undefined;
  private m_p1: vec2 | undefined = undefined;
  private m_p2: vec2 | undefined = undefined;
  private m_p3: vec2 | undefined = undefined;

  constructor() {
    this.id = nanoid();
    this.transform = new UntrackedTransform();
  }

  set({ p0, p1, p2, p3 }: { p0?: vec2; p1?: vec2; p2?: vec2; p3?: vec2 }): void {
    this.m_p0 = p0;
    this.m_p1 = p1;
    this.m_p2 = p2;
    this.m_p3 = p3;
  }

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel: boolean = false): void {}

  getDrawable(): Drawable {
    if (!this.m_p0 || !this.m_p3) return { operations: [] };
    const operations: DrawOp[] = [{ type: 'beginPath' }, { type: 'moveTo', data: [this.m_p0] }];

    if (this.m_p1 || this.m_p2) {
      if (this.m_p1 && this.m_p2)
        operations.push({ type: 'cubicTo', data: [this.m_p1, this.m_p2, this.m_p3] });
      else if (this.m_p1)
        operations.push({ type: 'cubicTo', data: [this.m_p1, this.m_p3, this.m_p3] });
      else if (this.m_p2)
        operations.push({ type: 'cubicTo', data: [this.m_p0, this.m_p2, this.m_p3] });
    } else operations.push({ type: 'lineTo', data: [this.m_p3] });

    operations.push({ type: 'stroke' });

    return { operations };
  }

  getOutlineDrawable = this.getDrawable;

  render(): void {
    Renderer.draw(this.getDrawable());
  }

  asObject(duplicate: boolean = false): GenericEntityObject {
    return {} as GenericEntityObject;
  }

  toJSON(): EntityObject {
    return this.asObject(false);
  }
}

export default Pen;
