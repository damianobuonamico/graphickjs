import { fillObject } from '@utils/utils';
import { vec2 } from '@math';
import Artboard from './ecs/artboard';
import ECS from './ecs/ecs';
import Element from './ecs/element';
import Layer from './ecs/layer';
import { Renderer } from './renderer';

abstract class SceneManager {
  private static m_ecs = new ECS();

  private static m_layer: Layer;

  public static viewport: ViewportState;

  public static init() {
    const artboard = new Artboard([600, 400]);
    this.m_layer = new Layer();
    const element = new Element();

    this.m_layer.add(element);
    artboard.add(this.m_layer);

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

  public static add(entity: Entity) {
    this.m_layer.add(entity);
  }

  public static render() {
    Renderer.beginFrame();
    this.m_ecs.render();
    Renderer.endFrame();
  }

  public static clientToScene(position: vec2, override: Partial<ViewportState> = {}) {
    const viewport = fillObject<ViewportState>(override, this.viewport);
    return vec2.sub(
      vec2.div(vec2.sub(position, Renderer.canvasOffset), viewport.zoom, true),
      viewport.position,
      true
    );
  }

  public static sceneToClient(position: vec2, override: Partial<ViewportState> = {}) {
    const viewport = fillObject<ViewportState>(override, this.viewport);
    return vec2.add(
      vec2.mul(vec2.add(position, viewport.position), viewport.zoom, true),
      Renderer.canvasOffset,
      true
    );
  }
}

export default SceneManager;
