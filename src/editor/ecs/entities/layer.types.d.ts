interface LayerEntity extends ECSEntity {
  readonly type: 'layer';
  readonly selectable: false;
  readonly transform: SimpleTransformComponent;
}

interface LayerOptions {
  id?: string;
}

interface LayerObject extends GenericEntityObject {
  children: EntityObject[];
}
