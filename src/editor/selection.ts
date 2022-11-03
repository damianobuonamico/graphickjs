import { vec2 } from '@/math';
import { debounce } from 'debounce';
import Color from './ecs/components/color';
import Fill from './ecs/components/fill';
import Stroke from './ecs/components/stroke';
import Element from './ecs/entities/element';
import Manipulator from './ecs/entities/manipulator';
import Vertex from './ecs/entities/vertex';
import { Renderer } from './renderer';
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

  static manipulator: Manipulator = new Manipulator();

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
      if (angle === null) angle = (entity.transform as TransformComponent).rotation ?? 0;
      else if (sameAngle && angle !== (entity.transform as TransformComponent).rotation)
        sameAngle = false;
    });

    return sameAngle ? angle : null;
  }

  static get boundingBox(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_selected.forEach((entity) => {
      if (entity.type !== 'element' || (entity as Element).length > 1) {
        const box = (entity.transform as TransformComponent).boundingBox;

        if (box) {
          box.forEach((point) => {
            vec2.min(min, point, min);
            vec2.max(max, point, max);
          });
        }
      }
    });

    return [min, max];
  }

  static get staticBoundingBox(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];

    this.m_selected.forEach((entity) => {
      if (entity.type !== 'element' || (entity as Element).length > 1) {
        const box = (entity.transform as TransformComponent).boundingBox;

        if (box) {
          box.forEach((point) => {
            const p = vec2.sub(point, (entity.transform as TransformComponent).positionDelta);
            vec2.min(min, p, min);
            vec2.max(max, p, max);
          });
        }
      }
    });

    return [min, max];
  }

  static get unrotatedBoundingBox(): Box {
    let min: vec2 = [Infinity, Infinity];
    let max: vec2 = [-Infinity, -Infinity];
    const angle = this.angle;

    if (angle) {
      this.m_selected.forEach((entity) => {
        if (entity.type !== 'element' || (entity as Element).length > 1) {
          const box = (entity.transform as TransformComponent).rotatedBoundingBox;

          if (box) {
            box.forEach((point) => {
              const rotated = vec2.rotate(point, [0, 0], -angle);
              vec2.min(min, rotated, min);
              vec2.max(max, rotated, max);
            });
          }
        }
      });

      const rMin = vec2.rotate(min, [0, 0], angle);
      const rMax = vec2.rotate(max, [0, 0], angle);

      const mid = vec2.mid(rMin, rMax);

      return [vec2.rotate(rMin, mid, -angle), vec2.rotate(rMax, mid, -angle)];
    } else {
      this.m_selected.forEach((entity) => {
        const box = (entity.transform as TransformComponent).boundingBox;

        if (box) {
          vec2.min(min, box[0], min);
          vec2.max(max, box[1], max);
        }
      });
    }

    return [min, max];
  }

  static calculateRenderOverlay() {
    if (!this.m_selected.size) this.manipulator.set(null);
    else {
      this.manipulator.set(this.unrotatedBoundingBox, this.angle ?? 0);
    }
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

    this.setProperties();
  }

  static deselect(id: string, deselectVertices = true) {
    if (deselectVertices) {
      const entity = this.m_selected.get(id);
      if (entity && entity.type === 'element') {
        (entity as Element).selection.clear();
      }
    }
    this.m_selected.delete(id);

    this.setProperties();
  }

  private static m_setFillProperty: (data: Partial<FillPropertyData>) => void = () => {};
  private static m_setStrokeProperty: (data: Partial<StrokePropertyData>) => void = () => {};

  static set setFillPropertyFn(fn: (data: Partial<FillPropertyData>) => void) {
    this.m_setFillProperty = fn;
  }

  static set setStrokePropertyFn(fn: (data: Partial<StrokePropertyData>) => void) {
    this.m_setStrokeProperty = fn;
  }

  private static setFillProperty() {
    const fill: FillPropertyData = { active: false, mixed: false, fills: [] };

    this.forEach((entity) => {
      if (entity.type === 'element') {
        if ((entity as Element).fill) {
          if (!fill.fills[0]) {
            fill.fills[0] = new Color((entity as Element).fill!.color.hex);
          } else if (!(entity as Element).fill!.color.equals(fill.fills[0])) {
            fill.mixed = true;
          }
        }
        fill.active = true;
      }
    });

    this.m_setFillProperty(fill);
  }

  private static setStrokeProperty() {
    const stroke: StrokePropertyData = { active: false, mixed: false, strokes: [] };

    this.forEach((entity) => {
      if (entity.type === 'element') {
        if ((entity as Element).stroke) {
          if (!stroke.strokes[0]) {
            stroke.strokes[0] = new Color((entity as Element).stroke!.color.hex);
          } else if (!(entity as Element).stroke!.color.equals(stroke.strokes[0])) {
            stroke.mixed = true;
          }
        }
        stroke.active = true;
      }
    });

    this.m_setStrokeProperty(stroke);
  }

  private static setPropertiesBody() {
    this.setFillProperty();
    this.setStrokeProperty();
  }

  private static setProperties = debounce(this.setPropertiesBody.bind(this), 2);

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
      if (entity.selectable) this.select(entity);
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

  static setFill({
    color,
    updateUI = true,
    commit = true
  }: {
    color?: string;
    updateUI?: boolean;
    commit?: boolean;
  }) {
    if (color) {
      if (commit) {
        this.forEach((entity) => {
          if (entity.type === 'element') {
            if ((entity as Element).fill) {
              (entity as Element).fill!.color.set(color);
            } else {
              (entity as Element).fill = new Fill({ color });
            }
          }
        });
      } else {
        this.forEach((entity) => {
          if (entity.type === 'element') {
            if ((entity as Element).fill) {
              (entity as Element).fill!.color.tempSet(color);
            } else {
              (entity as Element).fill = new Fill({ color });
            }
          }
        });
      }
      if (updateUI) this.m_setFillProperty({ fills: [new Color(color)] });
      SceneManager.render();
    } else if (commit) {
      this.forEach((entity) => {
        if (entity.type === 'element') {
          if ((entity as Element).fill) {
            (entity as Element).fill!.color.apply();
          }
        }
      });
    }
  }

  static setStroke({
    color,
    updateUI = true,
    commit = true
  }: {
    color?: string;
    updateUI?: boolean;
    commit?: boolean;
  }) {
    if (color) {
      if (commit) {
        this.forEach((entity) => {
          if (entity.type === 'element') {
            if ((entity as Element).stroke) {
              (entity as Element).stroke!.color.set(color);
            } else {
              (entity as Element).stroke = new Stroke({ color });
            }
          }
        });
      } else {
        this.forEach((entity) => {
          if (entity.type === 'element') {
            if ((entity as Element).stroke) {
              (entity as Element).stroke!.color.tempSet(color);
            } else {
              (entity as Element).stroke = new Stroke({ color });
            }
          }
        });
      }
      if (updateUI) this.m_setStrokeProperty({ strokes: [new Color(color)] });
      SceneManager.render();
    } else if (commit) {
      this.forEach((entity) => {
        if (entity.type === 'element') {
          if ((entity as Element).stroke) {
            (entity as Element).stroke!.color.apply();
          }
        }
      });
    }
  }
}

export { ElementSelectionManager };
export default SelectionManager;
