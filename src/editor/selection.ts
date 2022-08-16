import { Renderer } from './renderer';

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
    this.m_selected.clear();
    this.m_temp.clear();
  }

  public static select(entity: Entity) {
    this.m_selected.set(entity.id, entity);
  }

  public static deselect(id: string) {
    this.m_selected.delete(id);
  }

  public static temp(entities: Set<Entity>) {
    this.m_temp.clear();
    entities.forEach((entity) => {
      this.m_temp.set(entity.id, entity);
    });
  }

  public static sync() {
    this.m_temp.forEach((entity) => this.select(entity));
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
}

export default SelectionManager;
