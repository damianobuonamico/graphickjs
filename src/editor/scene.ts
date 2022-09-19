import { fillObject, stringifyReplacer } from '@utils/utils';
import { clamp, round, vec2 } from '@math';
import Artboard from './ecs/artboard';
import ECS from './ecs/ecs';
import Element from './ecs/element';
import Layer from './ecs/layer';
import { Renderer } from './renderer';
import Vertex from './ecs/vertex';
import HistoryManager from './history';
import {
  LOCAL_STORAGE_KEY,
  LOCAL_STORAGE_KEY_ASSETS,
  LOCAL_STORAGE_KEY_STATE,
  ZOOM_MAX,
  ZOOM_MIN
} from '@utils/constants';
import SelectionManager from './selection';
import InputManager from './input';
import { fileDialog } from '@/utils/file';
import Stroke from './ecs/components/stroke';
import Fill from './ecs/components/fill';
import AssetsManager from './ecs/assets';
import { parseSVG } from '@/utils/svg';

// DEV
import tigerSvg from '@utils/svg/demo';
import ImageEntity from './ecs/image';

abstract class SceneManager {
  private static m_ecs: ECS;
  private static m_renderOverlays: Map<string, Entity> = new Map();
  private static m_layer: Layer;

  public static viewport: ViewportState;
  public static assets: AssetsManager = new AssetsManager();

  public static setLoading: (loading: boolean) => void;

  public static init(setLoading: (loading: boolean) => void) {
    this.setLoading = setLoading;
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

  public static get(id: string) {
    return this.m_ecs.get(id);
  }

  public static add(entity: Entity) {
    this.m_layer.add(entity);
  }

  public static remove(entity: Entity, skipRecordAction = false) {
    (entity.parent as unknown as ECS).remove(entity.id, skipRecordAction);
  }

  public static delete(selected: Entity | true) {
    (selected === true ? SelectionManager.entities : [selected]).forEach((entity) => {
      if (
        entity.type === 'element' &&
        InputManager.tool.isVertex &&
        (entity as Element).selection.size < (entity as Element).vertexCount - 1
      ) {
        (entity as Element).selection.forEach((vertex) => {
          entity.delete(vertex);
        });
      } else {
        SelectionManager.deselect(entity.id);
        entity.parent.delete(entity);
        entity.deleteSelf();
      }
    });
  }

  public static render() {
    requestAnimationFrame(() => {
      Renderer.beginFrame();
      this.m_ecs.render();
      // TEMP: remove overlay rendering from selectionmanager
      SelectionManager.render(() => this.m_renderOverlays.forEach((entity) => entity.render()));
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
    localStorage.setItem(LOCAL_STORAGE_KEY_ASSETS, JSON.stringify(this.assets, stringifyReplacer));
  }

  public static load() {
    const state = localStorage.getItem(LOCAL_STORAGE_KEY_STATE);
    const data = localStorage.getItem(LOCAL_STORAGE_KEY);
    const assets = localStorage.getItem(LOCAL_STORAGE_KEY_ASSETS);

    this.m_ecs = new ECS();

    this.viewport = fillObject(state ? JSON.parse(state) : {}, {
      position: vec2.create(),
      zoom: 1,
      rotation: 0
    });

    if (assets) {
      this.assets.load(JSON.parse(assets) as AssetsObject);
    }

    if (data) {
      const parsed = JSON.parse(data) as EntityObject[];
      parsed.forEach((object) => {
        const entity = this.fromObject(object);
        if (entity) this.m_ecs.add(entity);
      });
    } else {
      const artboard = new Artboard({ size: [1920, 1080] });
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

    this.setLoading(false);

    // DEV
    // parseSVG(tigerSvg);
  }

  public static fromObject(object: EntityObject) {
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
      case 'image':
        return new ImageEntity({ ...(object as ImageObject) });
    }
  }

  public static getEntityAt(
    position: vec2,
    lowerLevel = false,
    threshold = 5 / SceneManager.viewport.zoom
  ) {
    return this.m_ecs.getEntityAt(position, lowerLevel, threshold);
  }

  public static getEntitiesIn(box: Box, lowerLevel = false) {
    const entities = new Set<Entity>();
    this.m_ecs.getEntitiesIn(box, entities, lowerLevel);
    return entities;
  }

  public static duplicate(entity: Entity) {
    const duplicate = this.fromObject(entity.toJSON(true));
    if (!duplicate) return undefined;
    this.add(duplicate);
    return duplicate;
  }

  public static pushRenderOverlay(entity: Entity) {
    this.m_renderOverlays.set(entity.id, entity);
  }

  public static hasRenderOverlay(id: string) {
    return this.m_renderOverlays.has(id);
  }

  public static popRenderOverlay(id?: string) {
    if (!id) {
      let order = Array.from(this.m_renderOverlays.keys());
      id = order[order.length - 1];
    }
    this.m_renderOverlays.delete(id);
  }

  public static clearRenderOverlays() {
    this.m_renderOverlays.clear();
    this.render();
  }

  public static forEach(callback: (entity: Entity) => any) {
    function forEachECS(entity: Entity) {
      callback(entity);
      if (entity.type !== 'element' && 'forEach' in entity) {
        (entity as any).forEach((e: Entity) => {
          forEachECS(e);
        });
      }
    }

    this.m_ecs.forEach((entity) => {
      forEachECS(entity);
    });
  }

  public static import() {
    fileDialog({ accept: ['.svg', '.png'], multiple: true }).then((files) => {
      this.setLoading(true);

      if (!files.length) {
        this.setLoading(false);
        return;
      }

      let current = 0;

      HistoryManager.beginSequence();
      SelectionManager.clear();

      Array.from(files).forEach((file) => {
        const reader = new FileReader();

        reader.onload = () => {
          if (!reader.result || typeof reader.result !== 'string') return;

          if (file.type === 'image/svg+xml') parseSVG(reader.result);
          else if (file.type === 'image/png')
            SceneManager.add(new ImageEntity({ source: reader.result }));

          current++;

          if (current === files.length) {
            HistoryManager.endSequence();
            this.render();
            this.setLoading(false);
          }
        };

        if (file.type === 'image/svg+xml') reader.readAsText(file);
        else if (file.type === 'image/png') reader.readAsDataURL(file);
      });
    });
  }
}

export default SceneManager;
