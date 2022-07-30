import Artboard from './ecs/artboard';
import ECS from './ecs/ecs';
import Element from './ecs/element';
import Layer from './ecs/layer';
import { Renderer } from './renderer';

abstract class SceneManager {
  private static m_ecs = new ECS();

  public static init() {
    const artboard = new Artboard([600, 400]);
    const layer = new Layer();
    const element = new Element();

    layer.add(element);
    artboard.add(layer);

    this.m_ecs.add(artboard);
  }

  public static render() {
    Renderer.beginFrame();
    this.m_ecs.forEach((entity) => {
      entity.render();
    });
    Renderer.endFrame();
  }
}

export default SceneManager;
