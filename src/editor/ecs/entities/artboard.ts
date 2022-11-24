import SceneManager from '@/editor/scene';
import { vec2 } from '@math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import LayerCompositing from '../components/layerCompositing';
import { RectTransform } from '../components/transform';
import ECS from '../ecs';

export const isArtboard = (b: Entity): b is Artboard => {
  return b.type === 'artboard';
};

class Artboard extends ECS implements ArtboardEntity {
  readonly id: string;
  readonly type = 'artboard';
  readonly selectable = false;
  readonly transform: RectTransform;
  readonly layer: LayerCompositingComponent;

  parent: Entity;

  private m_grid: GridType;

  constructor({ id = nanoid(), size, position = vec2.create(), grid = 'none' }: ArtboardOptions) {
    super();
    this.id = id;
    this.transform = new RectTransform(position, 0, size);
    this.layer = new LayerCompositing();
    this.m_grid = grid;
  }

  add(entity: Entity) {
    super.add(entity);
    entity.parent = this;
  }

  getDrawable(): Drawable {
    return {
      operations: [
        {
          type: 'rect',
          data: [this.transform.position.value, this.transform.size]
        },
        { type: 'fill' }
      ]
    };
  }

  getOutlineDrawable(): Drawable {
    return {
      operations: [
        {
          type: 'rect',
          data: [this.transform.position.value, this.transform.size]
        },
        { type: 'stroke' }
      ]
    };
  }

  render() {
    const size = this.transform.size;

    Renderer.rect({
      position: this.transform.position.value,
      size,
      fill: '#FFFFFF'
    });

    if (this.m_grid === 'rows') {
      let intensity = 200;
      const zoom = SceneManager.viewport.zoom;

      if (zoom < 1) intensity += (1 - zoom) * 40;

      const operations: DrawOp[] = [
        { type: 'strokeColor', data: [`rgb(${intensity}, ${intensity}, ${intensity})`] },
        { type: 'strokeWidth', data: [1 / zoom] },
        { type: 'beginPath' }
      ];

      const start = size[1] * 0.107744;
      const rowHeight = size[1] * 0.026936;
      const height = size[1] * 0.808081;

      for (let i = 0, n = Math.ceil(height / rowHeight); i < n; ++i) {
        operations.push({ type: 'moveTo', data: [[0, start + i * rowHeight]] });
        operations.push({ type: 'lineTo', data: [[size[0], start + i * rowHeight]] });
      }

      operations.push({ type: 'stroke' });
      Renderer.draw({ operations });
    }

    super.render();
  }

  asObject(duplicate = false): ArtboardObject {
    const object: ArtboardObject = {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: this.transform.position.value,
      size: this.transform.size,
      children: this.map((entity) => entity.asObject(duplicate))
    };

    if (this.m_grid !== 'none') object.grid = this.m_grid;

    return object;
  }

  toJSON() {
    return this.asObject(false);
  }
}

export default Artboard;
