interface ElementEntity extends TransformableEntity {
  // selection: ElementSelectionManagerComponent
  parent: LayerEntity;
  largeBoundingBox: Box;

  length: number;
  last: VertexEntity;
  vertices: VertexEntity[];

  recalculate(propagate?: boolean): void;
  regenerate(ids?: string[]): void;
  reverse(): void;
  forEach(callback: (vertex: VertexEntity, selected?: boolean) => void): void;

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
