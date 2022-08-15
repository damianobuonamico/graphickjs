type PenState = 'new' | 'join' | 'close' | 'sub' | 'add' | 'angle' | 'start';

interface PenToolData {
  element?: Entity;
  vertex?: Entity;
}

interface SelectToolData {
  element?: Entity;
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
}

type Tool = keyof ToolMap<any>;
