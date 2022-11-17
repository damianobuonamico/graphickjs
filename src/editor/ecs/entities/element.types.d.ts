interface ElementEntity extends Entity {
  readonly type: 'element';
  readonly selectable: true;
  readonly transform: ElementTransformComponent;
  readonly cache: CacheComponent;

  parent: LayerEntity;

  length: number;
  first: VertexEntity | undefined;
  last: VertexEntity | undefined;
  vertices: VertexEntity[];

  regenerate(ids?: string[]): void;
  reverse(): void;
  forEach(callback: (vertex: VertexEntity, selected: boolean, index: number) => void): void;
  forEachBezier(callback: (bezier: BezierEntity) => void): void;

  concat(element: ElementEntity): void;
  split(bezier: BezierEntity, position: vec2): VertexEntity | void;
  add(vertex: VertexEntity, regenerate?: boolean, index?: number): void;
  remove(vertex: VertexEntity | boolean, keepClosed?: boolean): void;
  close(mergeThreshold?: number): void;

  isOpenEnd(id: string): boolean;
  isFirstVertex(id: string): boolean;
  intersects(box: Box): boolean;
}

interface ElementOptions {
  id?: string;
  transform?: TransformComponentObject;
  position?: vec2;
  rotation?: number;
  vertices?: VertexEntity[];
  closed?: boolean;
  fill?: FillComponentObject;
  stroke?: StrokeComponentObject;
  recordHistory?: boolean;
}

interface ElementObject extends GenericEntityObject {
  transform: TransformComponentObject;
  vertices: VertexObject[];
  closed?: boolean;
  fill?: FillComponentObject;
  stroke?: StrokeComponentObject;
}
