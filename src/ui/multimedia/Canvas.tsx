import { Component, onMount, onCleanup } from "solid-js";
import { Renderer } from "@renderer";
import API from "@/wasm/loader";

const Canvas: Component = () => {
  onMount(() => {
    API._refresh();
  });

  onCleanup(() => {
    API._prepare_refresh();
  });

  return (
    <div class="flex grow overflow-hidden first-line:z-0">
      <canvas
        id="canvas"
        class="absolute overflow-hidden opacity-0"
        ref={(ref) => {
          if (ref !== null) {
            Renderer.setup(ref);
          }
        }}
      />
    </div>
  );
};

export default Canvas;
