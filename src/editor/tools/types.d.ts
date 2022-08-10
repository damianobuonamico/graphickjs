interface ToolData {
  callback(): PointerDownReturn;
  data?: PenToolData;
}

type PenState = 'new' | 'join' | 'close' | 'sub' | 'add' | 'angle' | 'start';

interface PenToolData {
  element?: Entity;
  vertex?: Entity;
}
