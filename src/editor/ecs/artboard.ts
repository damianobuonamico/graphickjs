import { nanoid } from 'nanoid';
import { Renderer } from '../renderer';
import ECS from './ecs';

class Artboard extends ECS implements Entity {
  public readonly id: string;

  private m_size: vec2;

  constructor(size: vec2) {
    super();
    this.id = nanoid();
    this.m_size = size;
  }

  public render() {
    Renderer.rect({ pos: [0, 0], size: this.m_size, color: [1.0, 1.0, 1.0, 1.0] });
    super.render();
  }
}

export default Artboard;
