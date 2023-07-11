import { Renderer } from "@/editor/renderer";
// const createModule = require("./editor.js");
// const createModule = await import("./editor.js");
// import wasm from "./editor";
// import createModule from "./editor";
// include("./editor.js");

declare global {
  function createModule(): Promise<any>;
}

const fallback: any = () => {};

const API: Api = {
  _init: fallback,
  _shutdown: fallback,
  _on_pointer_event: fallback,
  _on_keyboard_event: fallback,
  _on_resize_event: fallback,
  _on_wheel_event: fallback,
  _on_clipboard_event: fallback,
  _set_tool: fallback,
  _save: fallback,
  _load: fallback,
  _load_font: fallback,
  _load_svg: fallback,
  _to_heap: fallback,
  _free: fallback,

  // TEMP
  _render_frame: fallback,
  _translate_canvas: fallback,
  _scale_canvas: fallback,
  _install_vector_image: fallback,
  _download_and_install_vector_image: fallback,
};

createModule().then((module: any) => {
  // TEMP
  API._render_frame = module._RenderFrame;
  API._translate_canvas = module._TranslateCanvas;
  API._scale_canvas = module._ScaleCanvas;
  API._install_vector_image = module._InstallVectorImage;
  API._download_and_install_vector_image = (path: string) => {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", path, true);
    xhr.responseType = "arraybuffer";
    xhr.setRequestHeader("Access-Control-Allow-Origin", "*");

    xhr.onload = () => {
      if (xhr.status === 200) {
        const bytes = new Uint8Array(xhr.response);
        const count = bytes.length * bytes.BYTES_PER_ELEMENT;
        const allocation = module._malloc(count);
        console.log(xhr);

        module.HEAPU8.set(bytes, allocation);

        API._install_vector_image(allocation, count);

        module._free(allocation);

        API._render_frame();
      } else {
        // Error.
      }
    };

    xhr.send();
  };

  API._init = module._init || fallback;
  API._shutdown = module._shutdown || fallback;

  API._on_pointer_event = module._on_pointer_event || fallback;
  API._on_keyboard_event = module._on_keyboard_event || fallback;
  API._on_resize_event = module._on_resize_event || fallback;
  API._on_wheel_event = module._on_wheel_event || fallback;
  API._on_clipboard_event = module._on_clipboard_event || fallback;

  API._set_tool = module._set_tool || fallback;

  API._save = module._save || fallback;
  API._load = (data: string) => {
    // const ptr = module.allocateUTF8(data);
    // module._load(ptr);
    // module._free(ptr);
  };

  API._load_font = (data: ArrayBuffer) => {
    // const ptr = module._malloc(data.byteLength);
    // const heap = new Uint8Array(module.HEAPU8.buffer, ptr, data.byteLength);
    // heap.set(new Uint8Array(data));
    // module._load_font(ptr, data.byteLength);
    // module._free(ptr);
  };
  API._load_svg = (data: ArrayBuffer) => {
    // const ptr = module._malloc(data.byteLength);
    // const heap = new Uint8Array(module.HEAPU8.buffer, ptr, data.byteLength);
    // heap.set(new Uint8Array(data));
    // module._load_svg(ptr, data.byteLength);
    // module._free(ptr);
  };

  API._to_heap = (array: Float32Array) => {
    // const bytes = array.length * array.BYTES_PER_ELEMENT;

    // const dataPtr = module._malloc(bytes);
    // const dataHeap = new Uint8Array(module.HEAPU8.buffer, dataPtr, bytes);
    // dataHeap.set(new Uint8Array(array.buffer));

    // return dataHeap.byteOffset;
    return 0;
  };
  API._free = module._free;

  // module._init();
  Renderer.resize();

  fetch(
    "https://fonts.gstatic.com/s/roboto/v30/KFOmCnqEu92Fr1Mu4mxK.woff2"
  ).then((res) => {
    res.arrayBuffer().then((buffer) => {
      API._load_font(buffer);
    });
  });

  API._download_and_install_vector_image("Tiger.vectorimage");
});

export default API;
