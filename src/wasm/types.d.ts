interface Api {
  _init(): void;
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
    offsetX: number,
    offsetY: number
  ): boolean;
  _on_wheel_event(target: number, deltaX: number, deltaY: number): boolean;
  _on_clipboard_event(event: number): boolean;

  _to_heap(array: Float32Array): Pointer;
  _free(pointer: Pointer): void;
}

type Pointer = number;
