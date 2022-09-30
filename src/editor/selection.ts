import { vec2, vec3 } from '@/math';
import { GEOMETRY_MAX_ERROR, GEOMETRY_MAX_INTERSECTION_ERROR } from '@/utils/constants';
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
  private static m_angle: number | null = null;
  public static manipulator: Manipulator = new Manipulator();

  static get size() {
    return this.m_selected.size + this.m_temp.size;
  }

  static get ids() {
    return Array.from(this.m_selected.keys()).concat(Array.from(this.m_temp.keys()));
  }

  static get entities() {
    return Array.from(this.m_selected.values()).concat(Array.from(this.m_temp.values()));
  }

  static get angle(): number | null {
    let sameAngle = true;
    let angle: number | null = null;

    this.m_selected.forEach((entity) => {
      if (angle === null) angle = (entity as TransformableEntity).transform.rotation ?? 0;
      else if (sameAngle && angle !== (entity as TransformableEntity).transform.rotation)
        sameAngle = false;
    });

    return sameAngle ? angle : null;
  }

  static get boundingBox(): Box {
    // TODO: Cache
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];
    const angle = this.angle;

    if (angle) {
      this.m_selected.forEach((entity) => {
        if (entity.type !== 'element' || (entity as Element).length > 1) {
          const box = (entity as TransformableEntity).rotatedBoundingBox;

          box.forEach((point) => {
            const rotated = vec2.rotate(point, [0, 0], -angle);
            vec2.min(min, rotated, true);
            vec2.max(max, rotated, true);
          });
        }
      });

      const rMin = vec2.rotate(min, [0, 0], angle);
      const rMax = vec2.rotate(max, [0, 0], angle);

      const mid = vec2.div(vec2.add(rMin, rMax), 2);

      return [vec2.rotate(rMin, mid, -angle), vec2.rotate(rMax, mid, -angle)];
    } else {
      this.m_selected.forEach((entity) => {
        const box = (entity as MovableEntity).boundingBox;

        if (box) {
          vec2.min(min, box[0], true);
          vec2.max(max, box[1], true);
        }
      });
    }

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
    if (!this.m_selected.size) this.manipulator.set(null);
    else {
      this.manipulator.set(this.boundingBox, this.angle ?? 0);
    }
  }
}

export { ElementSelectionManager };
export default SelectionManager;
