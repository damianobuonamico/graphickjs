import { Component } from "solid-js";
import { Renderer } from "@renderer";

const Canvas: Component = () => {
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
