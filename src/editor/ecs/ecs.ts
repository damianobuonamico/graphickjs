import HistoryManager from '../history';
import SceneManager from '../scene';

class ECS {
  private m_children: Map<string, Entity>;
  private m_order: string[];

  constructor() {
    this.m_children = new Map();
    this.m_order = [];
  }

  public get(id: string) {
    return this.m_children.get(id);
  }

  private push(entity: Entity, index: number = this.m_order.length) {
    this.m_order.splice(index, 0, entity.id);
    this.m_children.set(entity.id, entity);
  }

  private splice(id: string, index?: number) {
    this.m_children.delete(id);
    this.m_order.splice(index || this.m_order.indexOf(id), 1);
  }

  public add(entity: Entity) {
    HistoryManager.record({
      fn: () => {
        this.push(entity);
      },
      undo: () => {
        this.splice(entity.id);
      }
    });
  }

  public remove(id: string, skipRecordAction = false) {
    const entity = this.m_children.get(id);
    if (!entity) return;
    const index = this.m_order.indexOf(id);

    if (skipRecordAction) {
      this.splice(id, index);
      return;
    }

    HistoryManager.record({
      fn: () => {
        this.splice(id);
      },
      undo: () => {
        this.push(entity, index);
      }
    });
  }

  public forEach(callback: (entity: Entity) => void) {
    this.m_order.forEach((id) => {
      callback(this.m_children.get(id)!);
    });
  }

  public forEachReversed(callback: (entity: Entity) => void) {
    for (let i = this.m_order.length - 1; i > -1; i--) {
      callback(this.m_children.get(this.m_order[i])!);
    }
  }

  public map<T>(callback: (entity: Entity) => T) {
    return this.m_order.map((id) => callback(this.m_children.get(id)!));
  }

  public render() {
    this.forEach((entity) => {
      entity.render();
    });
  }

  public getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    // Increase iterations when zoomed in
    let toReturn: Entity | undefined = undefined;
    this.forEachReversed((entity) => {
      if (!toReturn) {
        const result = entity.getEntityAt(position, lowerLevel, threshold);
        if (result) toReturn = result;
      }
    });
    return toReturn;
  }

  public getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel = false) {
    this.forEach((entity) => entity.getEntitiesIn(box, entities, lowerLevel));
  }
}

export default ECS;
