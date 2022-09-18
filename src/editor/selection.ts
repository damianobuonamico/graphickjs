import { isObject } from '@/utils/utils';
import Element from './ecs/element';
import Vertex from './ecs/vertex';
import InputManager from './input';
import { Renderer } from './renderer';
import SceneManager from './scene';

class ElementSelectionManager {
  private m_selected: Map<string, Vertex> = new Map();
  private m_temp: Map<string, Vertex> = new Map();
  private m_parent: Element;

  constructor(parent: Element) {
    this.m_parent = parent;
  }

  public get size() {
    return this.m_selected.size + this.m_temp.size;
  }

  public get ids() {
    return Array.from(this.m_selected.keys()).concat(Array.from(this.m_temp.keys()));
  }

  public get entities() {
    return Array.from(this.m_selected.values()).concat(Array.from(this.m_temp.values()));
  }

  public get full() {
    return this.m_selected.size === this.m_parent.vertexCount;
  }

  public has(id: string) {
    return this.m_selected.has(id) || this.m_temp.has(id);
  }

  public clear() {
    this.m_selected.clear();
    this.m_temp.clear();
    SelectionManager.deselect(this.m_parent.id, false);
  }

  public select(vertex: Vertex) {
    this.m_selected.set(vertex.id, vertex);
    if (!SelectionManager.has(this.m_parent.id)) SelectionManager.select(this.m_parent, false);
  }

  public deselect(id: string) {
    this.m_selected.delete(id);
    if (this.m_selected.size === 0) SelectionManager.deselect(this.m_parent.id);
  }

  public all() {
    this.m_parent.forEach((vertex) => {
      this.select(vertex);
    });
  }

  public temp(vertices: Set<Vertex>) {
    this.m_temp.clear();
    vertices.forEach((vertex) => {
      this.m_temp.set(vertex.id, vertex);
    });
  }

  public sync() {
    this.m_temp.forEach((vertex) => this.select(vertex));
    this.m_temp.clear();
  }

  public forEach(callback: (vertex: Vertex) => void) {
    this.m_selected.forEach((vertex) => callback(vertex));
    this.m_temp.forEach((vertex) => callback(vertex));
  }

  public get() {
    return Array.from(this.m_selected.values());
  }
}

abstract class SelectionManager {
  private static m_selected: Map<string, Entity> = new Map();
  private static m_temp: Map<string, Entity> = new Map();

  public static get size() {
    return this.m_selected.size + this.m_temp.size;
  }

  public static get ids() {
    return Array.from(this.m_selected.keys()).concat(Array.from(this.m_temp.keys()));
  }

  public static get entities() {
    return Array.from(this.m_selected.values()).concat(Array.from(this.m_temp.values()));
  }

  public static has(id: string) {
    return this.m_selected.has(id) || this.m_temp.has(id);
  }

  public static clear() {
    [this.m_selected, this.m_temp].forEach((map) => {
      map.forEach((entity) => {
        if (entity.type === 'element') (entity as Element).selection.clear();
      });
      map.clear();
    });
  }

  public static select(entity: Entity, selectVertices = true) {
    this.m_selected.set(entity.id, entity);
    if (selectVertices && entity.type === 'element') {
      (entity as Element).selection.all();
    }
  }

  public static deselect(id: string, deselectVertices = true) {
    if (deselectVertices) {
      const entity = this.m_selected.get(id);
      if (entity && entity.type === 'element') {
        (entity as Element).selection.clear();
      }
    }
    this.m_selected.delete(id);
  }

  public static temp(entities: Set<Entity>) {
    this.m_temp.clear();
    entities.forEach((entity) => {
      this.m_temp.set(entity.id, entity);
    });
  }

  public static sync(syncVertices = false) {
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

  public static forEach(callback: (entity: Entity) => void) {
    this.m_selected.forEach((entity) => callback(entity));
    this.m_temp.forEach((entity) => callback(entity));
  }

  public static render(overlay?: () => void) {
    Renderer.beginOutline();
    this.forEach((entity) => {
      Renderer.outline(entity);
    });
    if (overlay) overlay();
    Renderer.endOutline();
  }

  public static all() {
    SceneManager.forEach((entity) => {
      // TODO: Register selectable entities here
      if (entity.type === 'element' || entity.type === 'image') {
        this.select(entity);
      }
    });
  }

  public static get() {
    return Array.from(this.m_selected.values()).map((entity) => {
      if (entity.type === 'element') {
        return { element: entity, vertices: (entity as Element).selection.get() };
      }
      return entity;
    }) as SelectionBackup;
  }

  public static restore(selection: SelectionBackup) {
    this.clear();
    selection.forEach((entity) => {
      if (entity.hasOwnProperty('element') && entity.hasOwnProperty('vertices')) {
        const e = (entity as any).element as Entity;
        if (e.type === 'element') {
          (entity as any).vertices.forEach((vertex: Vertex) => {
            (e as Element).selection.select(vertex);
          });
        } else {
          this.select(e, false);
        }
      } else {
        this.select(entity as Entity, false);
      }
    });
  }
}

export { ElementSelectionManager };
export default SelectionManager;
