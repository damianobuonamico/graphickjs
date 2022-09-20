interface HandleEntity extends MovableEntity {
  handleType: HandleType;
}

type HandleType = 'vertex' | 'bezier';

interface HandleOptions {
  position: vec2;
  type: HandleType;
  parent: VertexEntity;
}

interface HandleObject extends GenericEntityObject {}
