type EntityType =
  | 'artboard'
  | 'layer'
  | 'element'
  | 'vertex'
  | 'handle'
  | 'bezier'
  | 'image'
  | 'manipulator'
  | 'generichandle';

interface Entity {
  readonly id: string;
  readonly type: EntityType;
  readonly selectable: boolean;

  parent: Entity;
  transform:
    | TransformComponent
    | SimpleTransformComponent
    | UntrackedTransformComponent
    | UntrackedSimpleTransformComponent;

  destroy(): void;

  getEntityAt(position: vec2, lowerLevel?: boolean, threshold?: number): Entity | undefined;
  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean): void;

  getDrawable(useWebGL?: boolean): Drawable;
  getOutlineDrawable(useWebGL?: boolean): Drawable;

  render(): void;

  asObject(duplicate?: boolean): EntityObject;
  toJSON(): EntityObject;
}

interface MovableEntity extends Entity {
  transform: SimpleTransformComponent;
  boundingBox: Box;
  staticBoundingBox: Box;
}

interface TransformableEntity extends Entity {
  transform: TransformComponent;
}

interface ECSEntity extends Entity {
  add(entity: Entity, skipRecordAction?: boolean): void;
  delete(entity: Entity, skipRecordAction?: boolean): void;
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
