import { nanoid } from 'nanoid';
import { Renderer } from '../renderer';
import ECS from './ecs';

class Layer extends ECS implements Entity {
  public readonly id: string;

  constructor() {
    super();
    this.id = nanoid();
  }

  public render() {
    Renderer.rect({ pos: [100, 100], size: [100, 100], color: [1.0, 1.0, 0.0, 1.0] });
    super.render();
  }
}

export default Layer;
