interface ArtboardEntity extends MovableEntity, ECSEntity {
  selectable: false;

  size: vec2;
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
