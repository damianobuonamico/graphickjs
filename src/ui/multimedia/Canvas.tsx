import { Component, createEffect } from "solid-js";
import { Renderer } from "@renderer";
import SceneManager from "@editor/scene";

const Canvas: Component = () => {
  let canvas: HTMLCanvasElement | undefined;

  createEffect(() => {
    if (canvas) {
      // TODO: Fix wasm loading timing issues
      setTimeout(() => {
        Renderer.resize();
        SceneManager.render();
        if (canvas) canvas.style.opacity = "1";
      }, 100);
    }
  });

  return (
    <div class="flex grow overflow-hidden first-line:z-0">
      <canvas
        class="absolute overflow-hidden"
        ref={(ref) => {
          if (ref !== null) Renderer.setup(ref);
          ref.style.opacity = "0";
          canvas = ref;
        }}
      ></canvas>
      <canvas
        id="canvas"
        class="absolute overflow-hidden"
        ref={(ref) => {
          if (ref !== null) Renderer.wasmCanvas = ref;
        }}
      />
    </div>
  );
};

export default Canvas;
