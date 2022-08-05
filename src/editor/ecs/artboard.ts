import { vec2 } from '@math';
import { nanoid } from 'nanoid';
import { Renderer } from '../renderer';
import ECS from './ecs';

class Artboard extends ECS implements Entity {
  public readonly id: string;
  public readonly type: Entity['type'] = 'artboard';
  public parent: Entity;

  private m_size: vec2;

  constructor({ id = nanoid(), size }: ArtboardOptions) {
    super();
    this.id = id;
    this.m_size = size;
  }

  public translate() {}

  public delete(entity: Entity) {
    this.remove(entity.id);
  }

  public add(entity: Entity) {
    super.add(entity);
    entity.parent = this;
  }

  public render() {
    Renderer.rect({ pos: [0, 0], size: this.m_size, color: [1.0, 1.0, 1.0, 1.0] });
    super.render();
  }

  public toJSON(duplicate = false) {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      size: vec2.clone(this.m_size),
      children: this.map((entity) => entity.toJSON(duplicate))
    };
  }
}

export default Artboard;
