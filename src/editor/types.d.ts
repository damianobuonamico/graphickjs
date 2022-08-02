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

interface Action {
  fn: () => void;
  undo: () => void;
}

interface PointerCoord {
  position: vec2;
  movement: vec2;
  delta: vec2;
  origin: vec2;
}

interface KeysState {
  ctrl: boolean;
  alt: boolean;
  shift: boolean;
  space: boolean;
  ctrlStateChanged: boolean;
  altStateChanged: boolean;
  shiftStateChanged: boolean;
  spaceStateChanged: boolean;
}

interface ToolState {
  current: Tool;
  active: Tool;
  pointerdown: (
    args: any
  ) => [
    (e: PointerEvent | TouchEvent) => void,
    (e: PointerEvent | TouchEvent) => void,
    (e: KeyboardEvent) => void
  ];
  pointermove?: (e: PointerEvent | TouchEvent) => void;
  pointerup?: (e: PointerEvent | TouchEvent, abort?: boolean) => void;
  keypress?: (e: KeyboardEvent) => void;
}

interface ViewportState {
  position: vec2;
  zoom: number;
  rotation: number;
}

interface Listeners {
  copy: (e: ClipboardEvent) => void;
  paste: (e: ClipboardEvent) => void;
  cut: (e: ClipboardEvent) => void;
  keydown: (e: KeyboardEvent) => void;
  keyup: (e: KeyboardEvent) => void;
  resize: (e: UIEvent) => void;
  unload: (e: Event) => void;
  pointerdown: (e: PointerEvent) => void;
  pointermove: (e: PointerEvent) => void;
  pointerup: (e: PointerEvent) => void;
  pointercancel: (e: PointerEvent) => void;
  pointerleave: (e: PointerEvent) => void;
  pointerenter: (e: PointerEvent) => void;
  wheel: (e: WheelEvent) => void;
}

interface MountedListener {
  type: keyof HTMLElementEventMap | keyof WindowEventMap;
  callback(e: Event): void;
  target: HTMLElement | Window | Document;
  options?: boolean | AddEventListenerOptions;
}
