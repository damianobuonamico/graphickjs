export type Mode = 'designer' | 'publisher' | 'photo';
export type Tool =
  | 'select'
  | 'vselect'
  | 'pen'
  | 'rectangle'
  | 'ellipse'
  | 'polygon'
  | 'pencil'
  | 'pan'
  | 'zoom'
  | 'eraser';

export interface State {
  mode: Mode;
  tool: Tool;
}
