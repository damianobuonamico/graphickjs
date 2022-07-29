import { Component, createEffect, onMount } from 'solid-js';

const Canvas: Component<{ canvas: Canvas }> = (props) => {
  let ref: HTMLDivElement | undefined = undefined;

  createEffect(() => {
    if (ref) props.canvas.container = ref;
  });

  return (
    <div class="flex grow overflow-hidden z-0" ref={ref}>
      <canvas
        class="absolute"
        ref={(ref) => {
          if (ref !== null) props.canvas.setup(ref);
        }}
      />
    </div>
  );
};

export default Canvas;
