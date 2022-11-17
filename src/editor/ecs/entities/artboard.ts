import { vec2 } from '@math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import { RectTransform } from '../components/transform';
import ECS from '../ecs';

class Artboard extends ECS implements ArtboardEntity {
  readonly id: string;
  readonly type = 'artboard';
  readonly selectable = false;

  parent: Entity;
  transform: RectTransform;

  constructor({ id = nanoid(), size, position = vec2.create() }: ArtboardOptions) {
    super();
    this.id = id;
    this.transform = new RectTransform(position, 0, size);
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
    Renderer.rect({
      position: this.transform.position.value,
      size: this.transform.size,
      fill: '#FFFFFF'
    });
    super.render();
  }

  asObject(duplicate = false): ArtboardObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: this.transform.position.value,
      size: this.transform.size,
      children: this.map((entity) => entity.asObject(duplicate))
    };
  }

  toJSON() {
    return this.asObject(false);
  }
}

export default Artboard;
