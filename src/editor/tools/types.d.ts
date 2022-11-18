type PenState = 'new' | 'join' | 'close' | 'sub' | 'add' | 'angle' | 'start';

interface PenDataStateInterface {
  readonly element: Value<ElementEntity | undefined>;
  readonly vertex: Value<VertexEntity | undefined>;
  readonly overlay: PenEntity;
}

interface PenToolData {
  pen?: PenDataStateInterface;
}

interface SelectToolData {
  selector?: SelectorEntity;
}

type ToolData = PenToolData | SelectToolData;

interface ToolMap<T> {
  select: T;
  vselect: T;
  pen: T;
  rectangle: T;
  ellipse: T;
  pan: T;
  zoom: T;
  scale: T;
  rotate: T;
}

type Tool = keyof ToolMap<any>;
