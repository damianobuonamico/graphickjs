import { Renderer } from '@/editor/renderer';
import SceneManager from '@/editor/scene';
import { nanoid } from 'nanoid';
import LayerCompositing from '../components/layerCompositing';
import { UntrackedTransform } from '../components/transform';

class Selector implements SelectorEntity {
  readonly id: string;
  readonly type = 'selector';
  readonly selectable = false;
  readonly transform: UntrackedTransform;
  readonly layer: LayerCompositingComponent;

  parent: Entity;

  private m_dashed: boolean;

  constructor({ position, dashed = false }: { position: vec2; dashed?: boolean }) {
    this.id = nanoid();
    this.transform = new UntrackedTransform(position);
    this.m_dashed = dashed;
    this.layer = new LayerCompositing();
  }

  set(size: vec2, position?: vec2): void {
    if (position) this.transform.position.value = position;
    this.transform.size = size;
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
    if (this.m_dashed)
      Renderer.draw({
        operations: [{ type: 'lineDash', data: [[3 / SceneManager.viewport.zoom]] }]
      });

    Renderer.rect({
      position: this.transform.position.value,
      size: this.transform.size,
      stroke: 'rgb(56, 195, 242)',
      fill: 'rgba(56, 195, 242, 0.2)'
    });

    Renderer.draw({ operations: [{ type: 'lineDash', data: [undefined] }] });
  }

  asObject(duplicate: boolean = false): GenericEntityObject {
    return {} as GenericEntityObject;
  }

  toJSON(): EntityObject {
    return this.asObject(false);
  }
}

export default Selector;
