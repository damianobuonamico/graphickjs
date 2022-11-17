import { OrderedMapValue } from '../history/value';

export const isECS = (b: Entity): b is ECSEntity => {
  return b instanceof ECS;
};

class ECS {
  private m_children: OrderedMapValue<string, Entity> = new OrderedMapValue();

  constructor() {}

  get = this.m_children.get.bind(this.m_children);

  add(entity: Entity) {
    this.m_children.set(entity.id, entity);
  }

  remove = this.m_children.delete.bind(this.m_children);

  forEach = this.m_children.forEach.bind(this.m_children);

  forEachReversed = this.m_children.forEachReversed.bind(this.m_children);

  map = this.m_children.map.bind(this.m_children);

  render() {
    this.forEach((entity) => {
      entity.render();
    });
  }

  getEntityAt(position: vec2, lowerLevel = false, threshold: number = 0) {
    let toReturn: Entity | undefined = undefined;

    this.forEachReversed((entity) => {
      if (!toReturn) {
        const result = entity.getEntityAt(position, lowerLevel, threshold);
        if (result) toReturn = result;
      }
    });

    return toReturn;
  }

  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel = false) {
    this.forEach((entity) => entity.getEntitiesIn(box, entities, lowerLevel));
  }
}

export default ECS;
