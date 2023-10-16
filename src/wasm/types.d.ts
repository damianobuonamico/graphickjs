interface Api {
  _init(): void;
  _prepare_refresh(): void;
  _refresh(): void;
  _shutdown(): void;

  _on_pointer_event(
    target: number,
    event: number,
    type: number,
    button: number,
    x: number,
    y: number,
    pressure: number,
    timeStamp: number,
    alt: boolean,
    ctrl: boolean,
    shift: boolean
  ): boolean;
  _on_keyboard_event(
    event: number,
    key: number,
    repeat: boolean,
    alt: boolean,
    ctrl: boolean,
    shift: boolean
  ): boolean;
  _on_resize_event(
    width: number,
    height: number,
    dpr: number,
    offsetX: number,
    offsetY: number
  ): boolean;
  _on_wheel_event(
    target: number,
    deltaX: number,
    deltaY: number,
    ctrl: boolean
  ): boolean;
  _on_clipboard_event(event: number): boolean;
  _on_touch_pinch(
    target: number,
    delta: number,
    center_x: number,
    center_y: number
  ): boolean;
  _on_touch_drag(target: number, deltaX: number, deltaY: number): boolean;

  _set_tool(type: number): void;

  _save(): string;
  _load(data: string): void;

  _load_font(data: ArrayBuffer): void;
  _load_svg(data: ArrayBuffer): void;

  _to_heap(array: Float32Array): Pointer;
  _free(pointer: Pointer): void;
}

type Pointer = number;
