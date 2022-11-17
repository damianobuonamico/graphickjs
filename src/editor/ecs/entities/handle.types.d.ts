interface HandleEntity extends Entity {
  readonly type: 'handle';
  readonly selectable: false;
  readonly transform: SimpleTransformComponent;

  handleType: HandleType;

  setCache(caches: CacheComponent): void;
}

type HandleType = 'vertex' | 'bezier';

interface HandleOptions {
  position: vec2;
  type: HandleType;
  parent: VertexEntity;
}

interface HandleObject extends GenericEntityObject {}
