import { fillObject } from '@utils/utils';
import { vec2 } from '@math';
import Artboard from './ecs/artboard';
import ECS from './ecs/ecs';
import Element from './ecs/element';
import Layer from './ecs/layer';
import { Renderer } from './renderer';
import Vertex from './ecs/vertex';
import HistoryManager from './history';
import { LOCAL_STORAGE_KEY, LOCAL_STORAGE_KEY_STATE } from '@utils/constants';

abstract class SceneManager {
  private static m_ecs: ECS;

  private static m_layer: Layer;

  public static viewport: ViewportState;

  public static init() {
    this.load();

    const artboard = new Artboard([600, 400]);
    this.m_layer = new Layer();
    const element = new Element({
      position: [100, 100],
      vertices: [
        new Vertex({ position: [0, 0] }),
        new Vertex({ position: [100, 0] }),
        new Vertex({ position: [100, 100] }),
        new Vertex({ position: [0, 100] })
      ]
    });

    this.m_layer.add(element);
    artboard.add(this.m_layer);

    this.m_ecs.add(artboard);

    HistoryManager.clear();
  }

  public static add(entity: Entity) {
    this.m_layer.add(entity);
  }

  public static remove(entity: Entity, skipRecordAction = false) {
    (entity.parent as unknown as ECS).remove(entity.id, skipRecordAction);
  }

  public static render() {
    requestAnimationFrame(() => {
      Renderer.beginFrame();
      this.m_ecs.render();
      Renderer.endFrame();
    });
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

  public static save() {
    //localStorage.setItem(LOCAL_STORAGE_KEY, JSON.stringify(this.m_ecs.asArray()));
    localStorage.setItem(
      LOCAL_STORAGE_KEY_STATE,
      JSON.stringify({
        ...this.viewport,
        position: [this.viewport.position[0], this.viewport.position[1]]
      })
    );
  }

  public static load() {
    const state = localStorage.getItem(LOCAL_STORAGE_KEY_STATE);

    this.m_ecs = new ECS();

    this.viewport = fillObject(state ? JSON.parse(state) : {}, {
      position: vec2.create(),
      zoom: 1,
      rotation: 0
    });
  }
}

export default SceneManager;
