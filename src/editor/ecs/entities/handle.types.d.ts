interface HandleEntity extends MovableEntity {
  selectable: false;

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
