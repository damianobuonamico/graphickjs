interface Entity {
  readonly id: string;
  readonly type: 'artboard' | 'layer' | 'element' | 'vertex' | 'handle';

  parent: Entity;

  render(): void;
  toJSON(): EntityObject;
}

type EntityObject = ArtboardObject | LayerObject | ElementObject | VertexObject;

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

interface HandleOptions {
  position: vec2;
  type: 'vertex' | 'bezier';
  parent: Entity;
}
