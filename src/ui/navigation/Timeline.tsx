import { Component, createEffect } from 'solid-js';
import { Button } from '../inputs';
import AnimationManager from '@/editor/animation/animation';
import { PauseIcon, PlayIcon, PlusIcon } from '../icons';
import SelectionManager from '@/editor/selection';

const Timeline: Component<{}> = (props) => {
  let canvas: HTMLCanvasElement | undefined;

  createEffect(() => {
    if (canvas) {
      AnimationManager.resize();
      canvas.style.opacity = '1';
    }
  });

  return (
    <div class="bg-primary-800 w-full h-full border-primary-600 border-t">
      <div class="flex w-full h-full">
        <canvas
          class="absolute"
          ref={(ref) => {
            if (ref !== null) AnimationManager.canvas = ref;
            ref.style.opacity = '0';
            canvas = ref;
          }}
        />

        <div class="z-50 flex h-fit">
          <Button onClick={() => AnimationManager.pause()}>
            <PauseIcon />
          </Button>
          <Button onClick={() => AnimationManager.play()}>
            <PlayIcon />
          </Button>
          <Button
            onClick={() => {
              SelectionManager.forEach((entity) => {
                AnimationManager.add(entity);
              });
            }}
          >
            <PlusIcon />
          </Button>
        </div>
      </div>
    </div>
  );
};

export default Timeline;
