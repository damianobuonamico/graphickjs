abstract class SelectionManager {
  private static m_selected: Map<string, Entity> = new Map();

  public static get size() {
    return this.m_selected.size;
  }

  public static get ids() {
    return Array.from(this.m_selected.keys());
  }

  public static get entities() {
    return Array.from(this.m_selected.values());
  }

  public static has(id: string) {
    return this.m_selected.has(id);
  }

  public static clear() {
    this.m_selected.clear();
  }

  public static select(entity: Entity) {
    this.m_selected.set(entity.id, entity);
  }

  public static deselect(id: string) {
    this.m_selected.delete(id);
  }

  public static sync() {}

  public static forEach(callback: (entity: Entity) => void) {
    this.m_selected.forEach((entity) => callback(entity));
  }
}

export default SelectionManager;
