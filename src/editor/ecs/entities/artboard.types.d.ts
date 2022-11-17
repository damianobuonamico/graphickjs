interface ArtboardEntity extends ECSEntity {
  readonly type: 'artboard';
  readonly selectable: false;
  readonly transform: SimpleTransformComponent;
}

interface ArtboardOptions {
  id?: string;
  position?: vec2;
  size: vec2;
}

interface ArtboardObject extends GenericEntityObject {
  position: vec2;
  size: vec2;
  children: EntityObject[];
}
