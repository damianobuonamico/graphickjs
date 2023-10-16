import { Renderer } from "@/editor/renderer";
import wasm from "./editor";

const fallback: any = () => {};

const API: Api = {
  _init: fallback,
  _prepare_refresh: fallback,
  _refresh: fallback,
  _shutdown: fallback,
  _on_pointer_event: fallback,
  _on_keyboard_event: fallback,
  _on_resize_event: fallback,
  _on_wheel_event: fallback,
  _on_clipboard_event: fallback,
  _on_touch_pinch: fallback,
  _on_touch_drag: fallback,
  _set_tool: fallback,
  _save: fallback,
  _load: fallback,
  _load_font: fallback,
  _load_svg: fallback,
  _to_heap: fallback,
  _free: fallback,
};

wasm().then((module: any) => {
  API._init = module._init;
  API._prepare_refresh = module._prepare_refresh;
  API._refresh = module._refresh;
  API._shutdown = module._shutdown;

  API._on_pointer_event = module._on_pointer_event;
  API._on_keyboard_event = module._on_keyboard_event;
  API._on_resize_event = module._on_resize_event;
  API._on_wheel_event = module._on_wheel_event;
  API._on_clipboard_event = module._on_clipboard_event;
  API._on_touch_pinch = module._on_touch_pinch;
  API._on_touch_drag = module._on_touch_drag;

  API._set_tool = module._set_tool;

  API._save = module._save;
  API._load = (data: string) => {
    const ptr = module.allocateUTF8(data);
    module._load(ptr);
    module._free(ptr);
  };

  API._load_font = (data: ArrayBuffer) => {
    const ptr = module._malloc(data.byteLength);
    const heap = new Uint8Array(module.HEAPU8.buffer, ptr, data.byteLength);
    heap.set(new Uint8Array(data));
    module._load_font(ptr, data.byteLength);
    module._free(ptr);
  };
  API._load_svg = (data: ArrayBuffer) => {
    const ptr = module._malloc(data.byteLength);
    const heap = new Uint8Array(module.HEAPU8.buffer, ptr, data.byteLength);
    heap.set(new Uint8Array(data));
    module._load_svg(ptr, data.byteLength);
    module._free(ptr);
  };

  API._to_heap = (array: Float32Array) => {
    const bytes = array.length * array.BYTES_PER_ELEMENT;

    const dataPtr = module._malloc(bytes);
    const dataHeap = new Uint8Array(module.HEAPU8.buffer, dataPtr, bytes);
    dataHeap.set(new Uint8Array(array.buffer));

    return dataHeap.byteOffset;
  };
  API._free = module._free;

  module._init();
  Renderer.resize();

  // fetch(
  //   "https://upload.wikimedia.org/wikipedia/commons/f/fd/Ghostscript_Tiger.svg"
  // )
  //   .then((res) => res.arrayBuffer())
  //   .then((text) => API._load_svg(text));

  // fetch(
  //   "https://fonts.gstatic.com/s/roboto/v30/KFOmCnqEu92Fr1Mu4mxK.woff2"
  // ).then((res) => {
  //   res.arrayBuffer().then((buffer) => {
  //     API._load_font(buffer);
  //   });
  // });
});

export default API;
