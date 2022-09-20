interface VertexEntity extends Entity {
  position: vec2;
  left: HandleEntity | undefined;
  right: HandleEntity | undefined;

  transform: SimpleTransformComponent;
  boundingBox: Box;

  setLeft(position?: vec2 | HandleEntity | null, skipRecordAction?: boolean): void;
  setRight(position?: vec2 | HandleEntity | null, skipRecordAction?: boolean): void;

  mirrorTranslation(id: string): void;
  applyMirroredTranslation(id: string): void;
  clearMirroredTranslation(id: string): void;

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
