type Mode = 'designer' | 'publisher' | 'photo';
type Tool =
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

interface State {
  mode: Mode;
  tool: Tool;
}
