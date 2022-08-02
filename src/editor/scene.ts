import { cloneObject, fillObject } from '@utils/utils';
import { vec2 } from '@math';
import Artboard from './ecs/artboard';
import ECS from './ecs/ecs';
import Element from './ecs/element';
import Layer from './ecs/layer';
import { Renderer } from './renderer';

abstract class SceneManager {
  private static m_ecs = new ECS();

  public static viewport: ViewportState;

  public static init() {
    const artboard = new Artboard([600, 400]);
    const layer = new Layer();
    const element = new Element();

    layer.add(element);
    artboard.add(layer);

    this.m_ecs.add(artboard);

    this.viewport = fillObject(
      {},
      {
        position: vec2.fromValues(0, 0),
        zoom: 1,
        rotation: 0
      }
    );
  }

  public static render() {
    Renderer.beginFrame();
    this.m_ecs.forEach((entity) => {
      entity.render();
    });
    Renderer.endFrame();
  }

  public static clientToScene(position: vec2) {
    return vec2.sub(
      vec2.div(vec2.sub(position, Renderer.canvasOffset), this.viewport.zoom, true),
      this.viewport.position,
      true
    );
  }

  public static sceneToClient(position: vec2) {
    return vec2.add(
      vec2.mul(vec2.add(position, this.viewport.position), this.viewport.zoom, true),
      Renderer.canvasOffset,
      true
    );
  }
}

export default SceneManager;
