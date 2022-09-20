import { isPointInCircle, vec2 } from '@math';
import { nanoid } from 'nanoid';
import HistoryManager from '../../history';
import InputManager from '../../input';
import { SimpleTransform } from '../components/transform';
import Element from './element';
import Vertex from './vertex';

class Handle implements HandleEntity {
  readonly id: string;
  readonly type: EntityType = 'handle';
  readonly handleType: HandleType;

  // TODO: check formatting in each entity file
  parent: VertexEntity;
  transform: SimpleTransformComponent;

  constructor({ position, type, parent }: HandleOptions) {
    this.id = nanoid();
    this.type = type;
    this.parent = parent;
    this.transform = new SimpleTransform(position);
  }

  get boundingBox(): Box {
    return [this.transform.position, this.transform.position];
  }

  destroy(): void {}

  getEntityAt(
    position: vec2,
    lowerLevel: boolean = false,
    threshold: number = 0
  ): Entity | undefined {
    if (isPointInCircle(position, this.transform.position, threshold)) return this;
    return undefined;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel: boolean = false): void {}

  getDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  getOutlineDrawable(useWebGL: boolean = false): Drawable {
    return { operations: [] };
  }

  render(): void {}

  asObject(duplicate: boolean = false): HandleObject {
    return { id: duplicate ? nanoid() : this.id, type: this.type };
  }

  toJSON(): EntityObject {
    return this.asObject(false);
  }
}

export default Handle;
