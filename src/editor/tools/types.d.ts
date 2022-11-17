type PenState = 'new' | 'join' | 'close' | 'sub' | 'add' | 'angle' | 'start';

interface PenToolData {
  element?: Entity;
  vertex?: VertexEntity;
  overlay?: Entity;
  overlayLastVertex?: VertexEntity;
  overlayVertex?: VertexEntity;
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
