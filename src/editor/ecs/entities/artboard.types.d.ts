interface ArtboardEntity extends ECSEntity {
  readonly type: 'artboard';
  readonly selectable: false;
  readonly transform: RectTransformComponent;
}

type GridType = 'none' | 'rows';

interface ArtboardOptions {
  id?: string;
  position?: vec2;
  size: vec2;
  grid?: GridType;
}

interface ArtboardObject extends GenericEntityObject {
  position: vec2;
  size: vec2;
  children: EntityObject[];
  grid?: GridType;
}
