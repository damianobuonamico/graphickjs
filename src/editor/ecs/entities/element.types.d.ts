interface ElementEntity extends TransformableEntity {
  parent: LayerEntity;
  selectable: true;

  readonly cache: any;

  length: number;
  last: VertexEntity;
  vertices: VertexEntity[];

  recalculate(propagate?: boolean): void;
  regenerate(ids?: string[]): void;
  reverse(): void;
  forEach(callback: (vertex: VertexEntity, selected?: boolean) => void): void;
  forEachBezier(callback: (bezier: BezierEntity) => void): void;

  concat(element: ElementEntity): void;
  split(bezier: BezierEntity, position: vec2): VertexEntity | void;
  push(vertex: VertexEntity, regenerate?: boolean, index?: number): void;
  close(mergeThreshold?: number): void;

  isOpenEnd(id: string): boolean;
  isFirstVertex(id: string): boolean;
  intersects(box: Box): boolean;

  delete(vertex: VertexEntity | boolean, keepClosed?: boolean): void;
}

interface ElementOptions {
  id?: string;
  position: vec2;
  rotation?: number;
  vertices?: VertexEntity[];
  closed?: boolean;
  stroke?: string | Stroke;
  fill?: string | Fill;
  recordHistory?: boolean;
}

interface ElementObject extends GenericEntityObject {
  position: vec2;
  vertices: VertexObject[];
  closed?: boolean;
  stroke?: string;
  fill?: string;
}
