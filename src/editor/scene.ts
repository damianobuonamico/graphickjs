import { fillObject, stringifyReplacer } from '@utils/utils';
import { clamp, round, vec2 } from '@math';
import Artboard from './ecs/entities/artboard';
import ECS from './ecs/ecs';
import Element, { isElement } from './ecs/entities/element';
import Layer from './ecs/entities/layer';
import { Renderer } from './renderer';
import Vertex from './ecs/entities/vertex';
import HistoryManager from './history';
import { LOCAL_STORAGE_KEY, LOCAL_STORAGE_KEY_STATE, ZOOM_MAX, ZOOM_MIN } from '@utils/constants';
import SelectionManager from './selection';
import InputManager from './input';
import { fileDialog } from '@/utils/file';
import Stroke from './ecs/components/stroke';
import Fill from './ecs/components/fill';
import { parseSVG } from '@/utils/svg';

// DEV
import tigerSvg from '@utils/svg/demo';
import ImageMedia from './ecs/entities/image';
import OverlayState from './overlays';
import Color from './ecs/components/color';
import AnimationManager from './animation';

abstract class SceneManager {
  private static m_ecs: ECS;
  private static m_layer: Layer;

  static viewport: ViewportState;
  static overlays: OverlayState = new OverlayState();

  static background = new Color([0.09, 0.11, 0.13, 1.0]);

  static setLoading: (loading: boolean) => void;

  static init(setLoading: (loading: boolean) => void) {
    this.setLoading = setLoading;
    this.load();
    HistoryManager.clear();
    AnimationManager.renderFn = this.renderFn.bind(this);
  }

  static set zoom(value: number | [number, vec2]) {
    const isArray = Array.isArray(value);
    const zoom = round(clamp(isArray ? value[0] : value, ZOOM_MIN, ZOOM_MAX), 4);
    if (isArray) {
      const delta = vec2.sub(
        this.clientToScene(vec2.clone(value[1]), { zoom }),
        this.clientToScene(vec2.clone(value[1]))
      );
      this.viewport.position = vec2.add(this.viewport.position, delta);
    }
    this.viewport.zoom = zoom;
  }

  static get(id: string) {
    return this.m_ecs.get(id);
  }

  static add(entity: Entity) {
    this.m_layer.add(entity);
  }

  static remove(entity: Entity, skipRecordAction = false) {
    (entity.parent as unknown as ECS).remove(entity.id, skipRecordAction);
  }

  static delete(selected: Entity | true, forceObject = false) {
    (selected === true ? SelectionManager.entities : [selected]).forEach((entity) => {
      if (
        !forceObject &&
        entity.type === 'element' &&
        InputManager.tool.isVertex &&
        (entity as Element).selection.size < (entity as Element).length - 1
      ) {
        (entity as Element).delete(true, false);
      } else {
        const backupSelection = (entity as Element).selection?.get();
        HistoryManager.record({
          fn: () => {
            SelectionManager.deselect(entity.id);
          },
          undo: () => {
            SelectionManager.select(entity);
            if (backupSelection) (entity as Element).selection?.restore(backupSelection);
          }
        });
        (entity.parent as ECSEntity).delete(entity);
        entity.destroy();
      }
    });
    SelectionManager.calculateRenderOverlay();
  }

  private static renderFn() {
    Renderer.beginFrame({
      color: this.background.hex,
      position: this.viewport.position,
      zoom: this.viewport.zoom
    });

    this.m_ecs.render();

    SelectionManager.render();

    this.overlays.render();

    Renderer.endFrame();
  }

  static render() {
    AnimationManager.render();
  }

  static clientToScene(position: vec2, override: Partial<ViewportState> = {}) {
    const viewport = fillObject<ViewportState>(override, this.viewport);
    const scene = vec2.create();

    vec2.sub(
      vec2.divS(vec2.sub(position, Renderer.canvasOffset, scene), viewport.zoom, scene),
      viewport.position,
      scene
    );

    return scene;
  }

  static sceneToClient(position: vec2, override: Partial<ViewportState> = {}) {
    const viewport = fillObject<ViewportState>(override, this.viewport);
    const client = vec2.create();

    vec2.add(
      vec2.mulS(vec2.add(position, viewport.position, client), viewport.zoom, client),
      Renderer.canvasOffset,
      client
    );

    return client;
  }

  static isVisible(entity: Entity) {
    const box = (entity.transform as TransformComponent).boundingBox;
    if (!box) return false;

    const position = this.viewport.position;
    const canvasSize = vec2.divS(Renderer.size, this.viewport.zoom);
    vec2.sub(canvasSize, this.viewport.position, canvasSize);

    return (
      box[1][0] >= -position[0] &&
      box[0][0] <= canvasSize[0] &&
      box[1][1] >= -position[1] &&
      box[0][1] <= canvasSize[1]
    );
  }

  static save() {
    localStorage.setItem(LOCAL_STORAGE_KEY_STATE, JSON.stringify(this.viewport, stringifyReplacer));
    localStorage.setItem(
      LOCAL_STORAGE_KEY,
      JSON.stringify(
        this.m_ecs.map((entity) => entity.toJSON()),
        stringifyReplacer
      )
    );
  }

  static load() {
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

    //this.add(new Demo({}));

    // DEV
    // parseSVG(tigerSvg);
  }

  // TODO: refactor and add transform
  static fromObject(object: EntityObject) {
    if (!object) return;

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
        return new ImageMedia({ ...(object as ImageObject) });
    }
  }

  static getEntityAt(
    position: vec2,
    lowerLevel = false,
    threshold = 5 / SceneManager.viewport.zoom
  ) {
    return (
      this.overlays.getEntityAt(position, lowerLevel, threshold) ??
      this.m_ecs.getEntityAt(position, lowerLevel, threshold)
    );
  }

  static getEntitiesIn(box: Box, lowerLevel = false) {
    const entities = new Set<Entity>();
    this.m_ecs.getEntitiesIn(box, entities, lowerLevel);
    return entities;
  }

  static duplicate(entity: Entity) {
    const duplicate = this.fromObject(entity.asObject(true));
    if (!duplicate) return undefined;
    this.add(duplicate);
    return duplicate;
  }

  static forEach(callback: (entity: Entity) => any) {
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

  static import() {
    fileDialog({ accept: ['.svg', '.png', '.jpg', '.jpeg'], multiple: true }).then((files) => {
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
          else if (file.type === 'image/png' || file.type === 'image/jpeg')
            SceneManager.add(new ImageMedia({ source: reader.result }));

          current++;

          if (current === files.length) {
            HistoryManager.endSequence();
            this.render();
            this.setLoading(false);
          }
        };

        if (file.type === 'image/svg+xml') reader.readAsText(file);
        else if (file.type === 'image/png' || file.type === 'image/jpeg')
          reader.readAsDataURL(file);
      });
    });
  }
}

export default SceneManager;
