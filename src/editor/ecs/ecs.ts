import HistoryManager from '../history';

class ECS {
  private m_children: Map<string, Entity>;
  private m_order: string[];

  constructor() {
    this.m_children = new Map();
    this.m_order = [];
  }

  private push(entity: Entity, index: number = this.m_order.length) {
    this.m_order.splice(this.m_order.length, 0, entity.id);
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
        this.splice(id, index);
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

  public render() {
    this.forEach((entity) => {
      entity.render();
    });
  }
}

export default ECS;
