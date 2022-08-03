import { fillObject, stringifyReplacer } from '@utils/utils';
import { clamp, round, vec2 } from '@math';
import Artboard from './ecs/artboard';
import ECS from './ecs/ecs';
import Element from './ecs/element';
import Layer from './ecs/layer';
import { Renderer } from './renderer';
import Vertex from './ecs/vertex';
import HistoryManager from './history';
import { LOCAL_STORAGE_KEY, LOCAL_STORAGE_KEY_STATE, ZOOM_MAX, ZOOM_MIN } from '@utils/constants';

abstract class SceneManager {
  private static m_ecs: ECS;

  private static m_layer: Layer;

  public static viewport: ViewportState;

  public static init() {
    this.load();
    HistoryManager.clear();
  }

  public static set zoom(value: number | [number, vec2]) {
    const isArray = Array.isArray(value);
    const zoom = round(clamp(isArray ? value[0] : value, ZOOM_MIN, ZOOM_MAX), 4);
    if (isArray) {
      const delta = vec2.sub(
        this.clientToScene(vec2.clone(value[1] as vec2) as vec2, { zoom }),
        this.clientToScene(vec2.clone(value[1] as vec2) as vec2)
      );
      this.viewport.position = vec2.add(this.viewport.position, delta);
    }
    this.viewport.zoom = zoom;
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
    localStorage.setItem(LOCAL_STORAGE_KEY_STATE, JSON.stringify(this.viewport, stringifyReplacer));
    localStorage.setItem(
      LOCAL_STORAGE_KEY,
      JSON.stringify(
        this.m_ecs.map((entity) => entity.toJSON()),
        stringifyReplacer
      )
    );
  }

  public static load() {
    const state = localStorage.getItem(LOCAL_STORAGE_KEY_STATE);
    const data = localStorage.getItem(LOCAL_STORAGE_KEY);

    this.m_ecs = new ECS();

    this.viewport = fillObject(state ? JSON.parse(state) : {}, {
      position: vec2.create(),
      zoom: 1,
      rotation: 0
    });

    if (data) {
      const parsed = JSON.parse(data) as EntityObject[];
      parsed.forEach((object) => {
        const entity = this.fromObject(object);
        if (entity) this.m_ecs.add(entity);
      });
    } else {
      const artboard = new Artboard({ size: [600, 400] });
      this.m_layer = new Layer({});
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
    }
  }

  private static fromObject(object: EntityObject) {
    switch (object.type) {
      case 'artboard': {
        const artboard = new Artboard({ ...(object as ArtboardObject) });
        (object as ArtboardObject).children.forEach((obj) => {
          const entity = this.fromObject(obj);
          if (entity) artboard.add(entity);
        });
        return artboard;
      }
      case 'layer':
        const layer = new Layer({ ...(object as LayerObject) });
        (object as LayerObject).children.forEach((obj) => {
          const entity = this.fromObject(obj);
          if (entity) layer.add(entity);
        });
        this.m_layer = layer;
        return layer;
      case 'element':
        const vertices: Vertex[] = [];
        (object as ElementObject).vertices.forEach((obj) => {
          const vertex = this.fromObject(obj);
          if (vertex) vertices.push(vertex as Vertex);
        });
        return new Element({ ...{ ...(object as ElementObject), vertices } });
      case 'vertex':
        return new Vertex({ ...(object as VertexObject) });
    }
  }
}

export default SceneManager;
