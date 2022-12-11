import { Component, createEffect } from 'solid-js';
import { Renderer } from '@renderer';

const Canvas: Component = () => {
  let canvas: HTMLCanvasElement | undefined;

  createEffect(() => {
    if (canvas) {
      Renderer.resize();
      canvas.style.opacity = '1';
    }
  });

  return (
    <div class="flex grow overflow-hidden first-line:z-0">
      <canvas
        id="canvas"
        class="absolute overflow-hidden"
        style={{ visibility: 'hidden' }}
        ref={(ref) => {
          if (ref !== null) Renderer.wasmCanvas = ref;
        }}
      />
      <canvas
        class="absolute overflow-hidden"
        ref={(ref) => {
          if (ref !== null) Renderer.setup(ref);
          ref.style.opacity = '0';
          canvas = ref;
        }}
      ></canvas>
    </div>
  );
};

export default Canvas;
