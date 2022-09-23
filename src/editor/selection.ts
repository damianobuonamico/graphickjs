import { vec2 } from '@/math';
import { isObject } from '@/utils/utils';
import Element from './ecs/entities/element';
import Manipulator from './ecs/entities/manipulator';
import Vertex from './ecs/entities/vertex';
import InputManager from './input';
import { Renderer } from './renderer';
import { createVertices } from './renderer/geometry';
import SceneManager from './scene';

class ElementSelectionManager {
  private m_selected: Map<string, VertexEntity> = new Map();
  private m_temp: Map<string, VertexEntity> = new Map();
  private m_parent: Element;

  constructor(parent: Element) {
    this.m_parent = parent;
  }

  get size() {
    return this.m_selected.size + this.m_temp.size;
  }

  get ids() {
    return Array.from(this.m_selected.keys()).concat(Array.from(this.m_temp.keys()));
  }

  get entities() {
    return Array.from(this.m_selected.values()).concat(Array.from(this.m_temp.values()));
  }

  get full() {
    return this.m_selected.size === this.m_parent.length;
  }

  has(id: string) {
    return this.m_selected.has(id) || this.m_temp.has(id);
  }

  clear() {
    this.m_selected.clear();
    this.m_temp.clear();
    SelectionManager.deselect(this.m_parent.id, false);
  }

  select(vertex: VertexEntity) {
    this.m_selected.set(vertex.id, vertex);
    if (!SelectionManager.has(this.m_parent.id)) SelectionManager.select(this.m_parent, false);
  }

  deselect(id: string) {
    this.m_selected.delete(id);
    if (this.m_selected.size === 0) SelectionManager.deselect(this.m_parent.id);
  }

  all() {
    this.m_parent.forEach((vertex) => {
      this.select(vertex);
    });
  }

  temp(vertices: Set<Vertex>) {
    this.m_temp.clear();
    vertices.forEach((vertex) => {
      this.m_temp.set(vertex.id, vertex);
    });
  }

  sync() {
    this.m_temp.forEach((vertex) => this.select(vertex));
    this.m_temp.clear();
  }

  forEach(callback: (vertex: VertexEntity) => void) {
    this.m_selected.forEach((vertex) => callback(vertex));
    this.m_temp.forEach((vertex) => callback(vertex));
  }

  get() {
    return Array.from(this.m_selected.values());
  }

  restore(selection: VertexEntity[]) {
    this.clear();
    selection.forEach((vertex) => {
      this.select(vertex);
    });
  }
}

abstract class SelectionManager {
  private static m_selected: Map<string, Entity> = new Map();
  private static m_temp: Map<string, Entity> = new Map();
  private static m_renderOverlay: Manipulator = new Manipulator();

  static get size() {
    return this.m_selected.size + this.m_temp.size;
  }

  static get ids() {
    return Array.from(this.m_selected.keys()).concat(Array.from(this.m_temp.keys()));
  }

  static get entities() {
    return Array.from(this.m_selected.values()).concat(Array.from(this.m_temp.values()));
  }

  static get boundingBox(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_selected.forEach((element) => {
      const box = (element as MovableEntity).boundingBox;

      if (box) {
        min = vec2.min(min, box[0]);
        max = vec2.max(max, box[1]);
      }
    });

    return [min, max];
  }

  static has(id: string) {
    return this.m_selected.has(id) || this.m_temp.has(id);
  }

  static clear() {
    [this.m_selected, this.m_temp].forEach((map) => {
      map.forEach((entity) => {
        if (entity.type === 'element') (entity as Element).selection.clear();
      });
      map.clear();
    });
  }

  static select(entity: Entity, selectVertices = true) {
    this.m_selected.set(entity.id, entity);
    if (selectVertices && entity.type === 'element') {
      (entity as Element).selection.all();
    }
  }

  static deselect(id: string, deselectVertices = true) {
    if (deselectVertices) {
      const entity = this.m_selected.get(id);
      if (entity && entity.type === 'element') {
        (entity as Element).selection.clear();
      }
    }
    this.m_selected.delete(id);
  }

  static temp(entities: Set<Entity>) {
    this.m_temp.clear();
    entities.forEach((entity) => {
      this.m_temp.set(entity.id, entity);
    });
  }

  static sync(syncVertices = false) {
    if (syncVertices) {
      this.m_temp.forEach((entity) => {
        if (entity.type === 'element') {
          (entity as Element).selection.sync();
          if ((entity as Element).selection.size) this.select(entity, false);
        }
      });
    } else {
      this.m_temp.forEach((entity) => this.select(entity));
    }
    this.m_temp.clear();
  }

  static forEach(callback: (entity: Entity) => void) {
    this.m_selected.forEach((entity) => callback(entity));
    this.m_temp.forEach((entity) => callback(entity));
  }

  static render(overlay?: () => void) {
    Renderer.beginOutline();
    this.forEach((entity) => {
      Renderer.outline(entity);
    });
    if (overlay) overlay();
    Renderer.endOutline();
  }

  static all() {
    SceneManager.forEach((entity) => {
      // TODO: Register selectable entities here
      if (entity.type === 'element' || entity.type === 'image') {
        this.select(entity);
      }
    });
  }

  static get() {
    return Array.from(this.m_selected.values()).map((entity) => {
      if (entity.type === 'element') {
        return { element: entity, vertices: (entity as Element).selection.get() };
      }
      return entity;
    }) as SelectionBackup;
  }

  static restore(selection: SelectionBackup) {
    this.clear();

    selection.forEach((entity) => {
      if (entity.hasOwnProperty('element') && entity.hasOwnProperty('vertices')) {
        const e = (entity as any).element as Entity;
        if (e.type === 'element') {
          (e as Element).selection.restore((entity as any).vertices);
        } else {
          this.select(e, false);
        }
      } else {
        this.select(entity as Entity, false);
      }
    });
  }

  static calculateRenderOverlay() {
    if (!this.m_selected.size) this.m_renderOverlay.set(null);
    else this.m_renderOverlay.set(this.boundingBox);
  }
}

export { ElementSelectionManager };
export default SelectionManager;
