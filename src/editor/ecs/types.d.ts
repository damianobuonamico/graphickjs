interface Entity {
  readonly id: string;
  readonly type: 'artboard' | 'layer' | 'element' | 'vertex' | 'handle' | 'bezier';

  parent: Entity;

  translate(delta: vec2): void;
  applyTransform(): void;
  delete(entity: Entity): void;

  render(): void;
  toJSON(duplicate?: boolean): EntityObject;
  getEntityAt(position: vec2, zoom: number): Entity | undefined;
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
}

interface ElementObject extends GenericEntityObject {
  position: vec2;
  vertices: VertexObject[];
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
  parent: Entity;
}

interface HandleObject extends GenericEntityObject {}

type BezierType = 'linear' | 'quadratic' | 'cubic';

interface BezierOptions {
  start: VertexEntity;
  end: VertexEntity;
}

interface BezierObject extends GenericEntityObject {}
