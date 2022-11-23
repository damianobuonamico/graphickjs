import { fillObject } from '@utils/utils';
import { clamp, round, vec2 } from '@math';
import Artboard from './ecs/entities/artboard';
import ECS, { isECS } from './ecs/ecs';
import Element from './ecs/entities/element';
import Layer from './ecs/entities/layer';
import { Renderer } from './renderer';
import Vertex from './ecs/entities/vertex';
import CommandHistory from './history/history';
import {
  LOCAL_STORAGE_KEY,
  LOCAL_STORAGE_KEY_SEQUENCE,
  LOCAL_STORAGE_KEY_STATE,
  ZOOM_MAX,
  ZOOM_MIN
} from '@utils/constants';
import SelectionManager from './selection';
import InputManager from './input';
import { fileDialog } from '@/utils/file';
import { parseSVG } from '@/utils/svg';
import ImageMedia from './ecs/entities/image';
import OverlayState from './overlays';
import Color from './ecs/components/color';
import AnimationManager from './animation/animation';
import Viewport from './viewport';

abstract class SceneManager {
  private static m_ecs: ECS;
  private static m_layer: Layer;

  static viewport: Viewport;
  static overlays: OverlayState = new OverlayState();

  static background = new Color([0.09, 0.11, 0.13, 1.0]);

  static state: Readonly<State>;
  static setLoading: (loading: boolean) => void;

  static init(state: State, setLoading: (loading: boolean) => void) {
    this.state = state;

    this.setLoading = setLoading;
    this.load();

    CommandHistory.clear();
    AnimationManager.renderFn = this.renderFn.bind(this);
  }

  static get(id: string) {
    return this.m_ecs.get(id);
  }

  static add(entity: Entity) {
    this.m_layer.add(entity);
  }

  static delete(selected: Entity | true, forceObject = false) {
    (selected === true ? SelectionManager.entities : [selected]).forEach((entity) => {
      if (
        !forceObject &&
        entity.type === 'element' &&
        InputManager.tool.isVertex &&
        (entity as Element).selection.size < (entity as Element).size - 1
      ) {
        (entity as Element).remove(true, false);
      } else {
        const backupSelection = (entity as Element).selection?.get();

        // TOCHECK
        SelectionManager.deselect(entity.id);

        // SelectionManager.select(entity);
        // if (backupSelection) (entity as Element).selection?.restore(backupSelection);

        if (!isECS(entity.parent)) return;
        entity.parent.remove(entity.id);
      }
    });

    SelectionManager.calculateRenderOverlay();
  }

  static setViewportArea() {
    this.viewport.viewport = Renderer.size;

    if (this.state.mode === 'whiteboard') {
      this.viewport.setBounds([[0, 0], this.m_layer.parent.transform.size]);
    } else {
      this.viewport.setBounds([
        [-Infinity, -Infinity],
        [Infinity, Infinity]
      ]);
    }

    this.viewport.zoom = this.viewport.zoom || 1;
    this.viewport.position = this.viewport.position || [0, 0];
  }

  static onWorkspaceChange(workspace: Workspace) {
    this.setViewportArea();
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

  static clientToScene(position: vec2) {
    return this.viewport.clientToLocal(position, Renderer.canvasOffset);
  }

  static sceneToClient(position: vec2) {
    return this.viewport.localToClient(position, Renderer.canvasOffset);
  }

  static isVisible(entity: Entity) {
    const box = entity.transform.boundingBox;
    if (!box) return false;

    return this.viewport.isVisible(box, Renderer.size);
  }

  static save() {
    localStorage.setItem(LOCAL_STORAGE_KEY_STATE, JSON.stringify(this.viewport));
    localStorage.setItem(LOCAL_STORAGE_KEY_SEQUENCE, JSON.stringify(AnimationManager.toJSON()));
    localStorage.setItem(
      LOCAL_STORAGE_KEY,
      JSON.stringify(this.m_ecs.map((entity) => entity.toJSON()))
    );
  }

  static load() {
    const sequence = localStorage.getItem(LOCAL_STORAGE_KEY_SEQUENCE);
    const state = localStorage.getItem(LOCAL_STORAGE_KEY_STATE);
    const data = localStorage.getItem(LOCAL_STORAGE_KEY);

    this.m_ecs = new ECS();

    const viewport = state ? JSON.parse(state) : {};
    this.viewport = new Viewport(viewport.position, viewport.zoom, viewport.rotation);

    if (data) {
      const parsed = JSON.parse(data) as EntityObject[];
      parsed.forEach((object) => {
        const entity = this.fromObject(object);
        if (entity) this.m_ecs.add(entity);
      });
    } else {
      const artboard = new Artboard({ size: [1080, 1920] });
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

    if (sequence && sequence !== 'undefined') {
      let entitiesMap: Map<string, Entity> = new Map();
      let seq = JSON.parse(sequence);
      let nodes: Set<string> = new Set(seq.nodes);
      this.forEach((entity) => {
        if (nodes.has(entity.id)) entitiesMap.set(entity.id, entity);
      });
      AnimationManager.load(Array.from(entitiesMap.values()));
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

        if (!vertices.length) return;

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

      SelectionManager.clear();

      Array.from(files).forEach((file) => {
        const reader = new FileReader();

        reader.onload = () => {
          if (!reader.result || typeof reader.result !== 'string') return;

          if (file.type === 'image/svg+xml') {
            CommandHistory.endBatch();

            console.time('svg');
            let entities = parseSVG(reader.result);
            console.timeEnd('svg');

            if (entities) {
              if (!Array.isArray(entities)) entities = [entities];
              CommandHistory.pop();

              for (let i = 0, n = entities.length; i < n; ++i) {
                SceneManager.add(entities[i]);
              }
            }
          } else if (file.type === 'image/png' || file.type === 'image/jpeg') {
            SceneManager.add(new ImageMedia({ source: reader.result }));
          }

          current++;

          if (current === files.length) {
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
