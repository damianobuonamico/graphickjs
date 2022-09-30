interface VertexEntity extends Entity {
  parent: ElementEntity;

  position: HandleEntity;
  left: HandleEntity | undefined;
  right: HandleEntity | undefined;

  transform: VertexTransformComponent;
  boundingBox: Box;

  getEntitiesIn(
    box: Box,
    entities: Set<Entity>,
    lowerLevel?: boolean,
    angle?: number,
    center?: vec2
  ): void;

  render(selected?: boolean): void;
}

interface VertexOptions {
  id?: string;
  position: vec2;
  left?: vec2;
  right?: vec2;
}

interface VertexObject extends GenericEntityObject {
  position: vec2;
  left?: vec2;
  right?: vec2;
}
