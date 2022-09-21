import { Component, createEffect, onMount } from 'solid-js';
import { Renderer } from '@renderer';

const Canvas: Component = (props) => {
  let ref: HTMLDivElement | undefined = undefined;

  createEffect(() => {
    if (ref) Renderer.container = ref;
  });

  return (
    <div class="flex grow overflow-hidden z-0" ref={ref}>
      <canvas
        class="absolute"
        ref={(ref) => {
          if (ref !== null) Renderer.setup(ref);
        }}
      />
    </div>
  );
};

export default Canvas;
