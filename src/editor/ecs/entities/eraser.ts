import { Renderer } from '@/editor/renderer';
import SceneManager from '@/editor/scene';
import { nanoid } from 'nanoid';
import LayerCompositing from '../components/layerCompositing';
import { UntrackedTransform } from '../components/transform';

class Eraser implements EraserEntity {
  readonly id: string;
  readonly type = 'eraser';
  readonly selectable = false;
  readonly transform: UntrackedTransform;
  readonly layer: LayerCompositingComponent;

  parent: Entity;

  private m_radius: number = 1;

  constructor({ position }: { position: vec2 }) {
    this.id = nanoid();
    this.transform = new UntrackedTransform(position);
    this.layer = new LayerCompositing();
  }

  set(position: vec2, radius: number = 1): void {
    this.transform.position.value = position;
    this.m_radius = radius;
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
    return { operations: [] };
  }

  getOutlineDrawable = this.getDrawable;

  render(): void {
    const operations: DrawOp[] = [
      { type: 'strokeColor', data: ['#AAA'] },
      { type: 'beginPath' },
      { type: 'circle', data: [this.transform.position.value, this.m_radius] },
      { type: 'stroke' }
    ];

    Renderer.draw({ operations });
  }

  asObject(duplicate: boolean = false): GenericEntityObject {
    return {} as GenericEntityObject;
  }

  toJSON(): EntityObject {
    return this.asObject(false);
  }
}

export default Eraser;
