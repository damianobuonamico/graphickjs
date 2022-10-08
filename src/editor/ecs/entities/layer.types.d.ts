interface LayerEntity extends ECSEntity {
  selectable: false;
}

interface LayerOptions {
  id?: string;
}

interface LayerObject extends GenericEntityObject {
  children: EntityObject[];
}
