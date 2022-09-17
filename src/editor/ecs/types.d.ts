interface Entity {
  readonly id: string;
  readonly type: 'artboard' | 'layer' | 'element' | 'vertex' | 'handle' | 'bezier';

  parent: Entity;
  visible: boolean;
  position: vec2;

  move(delta: vec2): void;
  moveTo(position: vec2): void;
  translate(delta: vec2): void;
  applyTransform(): void;
  clearTransform(): void;

  getEntityAt(position: vec2, lowerLevel: boolean, zoom: number): Entity | undefined;
  getEntitiesIn(box: Box, entities: Set<Entity>, lowerLevel?: boolean): void;

  delete(entity: Entity): void;
  deleteSelf(): void;

  render(): void;

  toJSON(duplicate?: boolean): EntityObject;
}

type EntityObject = ArtboardObject | LayerObject | ElementObject | VertexObject | HandleObject;

interface GenericEntityObject {
  id: string;
  type: Entity['type'];
}

interface ArtboardOptions {
  id?: string;
  size: vec2;
}

interface ArtboardObject extends GenericEntityObject {
  size: vec2;
  children: EntityObject[];
}

interface LayerOptions {
  id?: string;
}

interface LayerObject extends GenericEntityObject {
  children: EntityObject[];
}

interface ElementOptions {
  id?: string;
  position: vec2;
  vertices?: VertexEntity[];
  closed?: boolean;
  stroke?: string | Stroke;
  fill?: string | Fill;
}

interface ElementObject extends GenericEntityObject {
  position: vec2;
  vertices: VertexObject[];
  closed?: boolean;
  stroke?: string;
  fill?: string;
}

interface VertexEntity extends Entity {}

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

type HandleType = 'vertex' | 'bezier';

interface HandleOptions {
  position: vec2;
  type: HandleType;
  parent: VertexEntity;
}

interface HandleObject extends GenericEntityObject {}

type BezierType = 'linear' | 'cubic';

interface BezierOptions {
  start: VertexEntity;
  end: VertexEntity;
}

interface BezierObject extends GenericEntityObject {}
