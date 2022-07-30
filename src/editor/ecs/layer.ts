import { nanoid } from 'nanoid';
import ECS from './ecs';

class Layer extends ECS implements Entity {
  public readonly id: string;

  constructor() {
    super();
    this.id = nanoid();
  }

  public render() {}
}

export default Layer;
