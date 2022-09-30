import { vec2 } from '@math';
import { nanoid } from 'nanoid';
import { Renderer } from '../../renderer';
import { SimpleTransform } from '../components/transform';
import ECS from '../ecs';

class Artboard extends ECS implements ArtboardEntity {
  public readonly id: string;
  public readonly type: EntityType = 'artboard';

  public parent: Entity;
  public transform: SimpleTransformComponent;

  private m_size: vec2;

  constructor({ id = nanoid(), size, position = vec2.create() }: ArtboardOptions) {
    super();
    this.id = id;
    this.m_size = size;
    this.transform = new SimpleTransform(position);
  }

  public get boundingBox(): Box {
    return [this.transform.position, vec2.add(this.transform.position, this.m_size, true)];
  }

  public get staticBoundingBox(): Box {
    return [
      this.transform.staticPosition,
      vec2.add(this.transform.staticPosition, this.m_size, true)
    ];
  }

  public get size() {
    return vec2.clone(this.m_size);
  }

  public add(entity: Entity) {
    super.add(entity);
    entity.parent = this;
  }

  public delete(entity: Entity) {
    this.remove(entity.id);
  }

  public destroy(): void {}

  public getDrawable(useWebGL: boolean = false): Drawable {
    return {
      operations: [
        {
          type: 'rect',
          data: [this.transform.position, this.m_size]
        },
        { type: 'fill' }
      ]
    };
  }

  public getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return {
      operations: [
        {
          type: 'rect',
          data: [this.transform.position, this.m_size]
        },
        { type: 'stroke' }
      ]
    };
  }

  public render() {
    // TODO: refactor renderer
    Renderer.rect({ pos: [0, 0], size: this.m_size, color: [1.0, 1.0, 1.0, 1.0] });
    super.render();
  }

  public asObject(duplicate = false): ArtboardObject {
    return {
      id: duplicate ? nanoid() : this.id,
      type: this.type,
      position: this.transform.position,
      size: this.size,
      children: this.map((entity) => entity.asObject(duplicate))
    };
  }

  public toJSON() {
    return this.asObject(false);
  }
}

export default Artboard;
