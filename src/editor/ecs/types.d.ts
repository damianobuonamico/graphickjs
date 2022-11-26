type EntityType =
  | 'artboard'
  | 'layer'
  | 'element'
  | 'vertex'
  | 'handle'
  | 'bezier'
  | 'image'
  | 'manipulator'
  | 'generichandle'
  | 'selector'
  | 'pen'
  | 'freehand';

interface Entity {
  readonly id: string;
  readonly type: EntityType;
  readonly selectable: boolean;
  readonly transform: GenericTransformComponent;
  readonly layer: LayerCompositingComponent;

  parent: Entity;

  getEntityAt(position: vec2, lowerLevel?: boolean, threshold?: number): Entity | undefined;
  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean): void;

  getDrawable(useWebGL?: boolean): Drawable;
  getOutlineDrawable(useWebGL?: boolean): Drawable;

  render(): void;

  asObject(duplicate?: boolean): EntityObject;
  toJSON(): EntityObject;
}

interface TransformableEntity extends Entity {
  transform: TransformComponent;
}

interface ECSEntity extends Entity {
  add(entity: Entity): void;
  remove(id: string): void;
}

type EntityObject =
  | ArtboardObject
  | LayerObject
  | ElementObject
  | VertexObject
  | HandleObject
  | BezierObject;

interface GenericEntityObject {
  id: string;
  type: EntityType;
}
