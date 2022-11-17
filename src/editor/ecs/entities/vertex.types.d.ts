interface VertexEntity extends Entity {
  readonly type: 'vertex';
  readonly selectable: false;
  readonly transform: VertexTransformComponent;

  parent: ElementEntity;

  position: HandleEntity;
  left: HandleEntity | undefined;
  right: HandleEntity | undefined;

  registerCache(cache: CacheComponent): void;
  pauseCache(): void;

  getEntitiesIn(
    box: Box,
    entities: Set<Entity>,
    lowerLevel?: boolean,
    angle?: number,
    center?: vec2
  ): void;
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
