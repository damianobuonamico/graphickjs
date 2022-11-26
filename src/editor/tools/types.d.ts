type PenState = 'new' | 'join' | 'close' | 'sub' | 'add' | 'angle' | 'start';

interface PenToolStateInterface {
  readonly element: Value<ElementEntity | undefined>;
  readonly vertex: Value<VertexEntity | undefined>;
  readonly overlay: PenEntity;
}

interface PenToolData {
  pen?: PenToolStateInterface;
}

interface DirectSelectToolStateInterface {
  draggingOccurred: boolean;
  entityIsAddedToSelection: boolean;
}

interface SelectToolData {
  selector?: SelectorEntity;
}

type ToolData = PenToolData | SelectToolData;

interface ToolMap<T> {
  select: T;
  directSelect: T;
  pen: T;
  rectangle: T;
  ellipse: T;
  pan: T;
  zoom: T;
  scale: T;
  rotate: T;
  pencil: T;
  eraser: T;
}

type Tool = keyof ToolMap<any>;
