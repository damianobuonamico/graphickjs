import wasm from "./editor";

const fallback: any = () => {};

const API: Api = {
  _init: fallback,
  _shutdown: fallback,
  _on_pointer_event: fallback,
  _on_keyboard_event: fallback,
  _on_resize_event: fallback,
  _on_wheel_event: fallback,
  _on_clipboard_event: fallback,
  _to_heap: fallback,
  _free: fallback,
};

wasm().then((module: any) => {
  API._init = module._init;
  API._shutdown = module._shutdown;

  API._on_pointer_event = module._on_pointer_event;
  API._on_keyboard_event = module._on_keyboard_event;
  API._on_resize_event = module._on_resize_event;
  API._on_wheel_event = module._on_wheel_event;
  API._on_clipboard_event = module._on_clipboard_event;

  API._to_heap = (array: Float32Array) => {
    const bytes = array.length * array.BYTES_PER_ELEMENT;

    const dataPtr = module._malloc(bytes);
    const dataHeap = new Uint8Array(module.HEAPU8.buffer, dataPtr, bytes);
    dataHeap.set(new Uint8Array(array.buffer));

    return dataHeap.byteOffset;
  };
  API._free = module._free;

  module._init();
});

export default API;
