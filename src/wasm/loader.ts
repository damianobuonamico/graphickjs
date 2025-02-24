import { Renderer } from '@/editor/renderer';
import wasm from './editor';

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
  _ui_data: fallback,
  _modify_ui_data: fallback,
  _save: fallback,
  _load: fallback,
  _load_font: fallback,
  _load_svg: fallback,
  _load_image: fallback,
  _to_heap: fallback,
  _free: fallback
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
  API._ui_data = () => {
    const str_ptr: number = module._ui_data();
    const str: string = module.UTF8ToString(str_ptr);

    module._free(str_ptr);

    return JSON.parse(str);

    // const json: object = JSON.parse(str);

    // console.log(json);

    // return json;

    // const data = module._ui_data();

    // console.log(data.size());
    // console.log(data.get(1));

    // return {
    //   data: 0,
    //   size: 0
    // };
  };
  API._modify_ui_data = (data: object) => {
    const str = JSON.stringify(data);
    const ptr = module.stringToNewUTF8(str);

    module._modify_ui_data(ptr);
    module._free(ptr);
  };

  API._save = module._save;
  API._load = (data: string) => {
    const ptr = module.stringToNewUTF8(data);
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
    module._load_svg(ptr);
    module._free(ptr);
  };
  API._load_image = (data: ArrayBuffer) => {
    const ptr = module._malloc(data.byteLength);
    const heap = new Uint8Array(module.HEAPU8.buffer, ptr, data.byteLength);
    heap.set(new Uint8Array(data));
    module._load_image(ptr, data.byteLength);
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

  fetch('https://upload.wikimedia.org/wikipedia/commons/f/fd/Ghostscript_Tiger.svg')
    .then((res) => res.arrayBuffer())
    .then((text) => API._load_svg(text));

  fetch('https://upload.wikimedia.org/wikipedia/it/thumb/e/ea/Dart_Fener.jpg/1024px-Dart_Fener.jpg')
    .then((res) => res.arrayBuffer())
    .then((text) => API._load_image(text));

  setTimeout(() => {
    const buffer = API._ui_data();
    console.log(buffer);

    // API._free(buffer.data);
  }, 1000);

  // fetch(
  //   "https://fonts.gstatic.com/s/roboto/v30/KFOmCnqEu92Fr1Mu4mxK.woff2"
  // ).then((res) => {
  //   res.arrayBuffer().then((buffer) => {
  //     API._load_font(buffer);
  //   });
  // });
});

export default API;
