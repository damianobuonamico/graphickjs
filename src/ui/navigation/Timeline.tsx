import { Component, createEffect } from 'solid-js';
import { Button } from '../inputs';
import AnimationManager from '@/editor/animation/animation';
import { PauseIcon, PlayIcon, PlusIcon, StopIcon } from '../icons';
import SelectionManager from '@/editor/selection';

const Timeline: Component<{ onResize: (movement: number) => void }> = (props) => {
  let canvas: HTMLCanvasElement | undefined;

  createEffect(() => {
    if (canvas) {
      AnimationManager.resize();
      canvas.style.opacity = '1';
    }
  });

  const onPointerDown = () => {
    window.addEventListener('pointermove', onPointerMove);
    window.addEventListener('pointerup', onPointerUp);
  };

  const onPointerMove = (e: PointerEvent) => props.onResize(e.clientY);

  const onPointerUp = () => {
    window.removeEventListener('pointermove', onPointerMove);
    window.removeEventListener('pointerup', onPointerUp);
  };

  return (
    <div class="bg-primary-800 w-full h-full border-primary-600 border-t">
      <div class="w-full h-0 z-[1000]">
        <div
          class="w-full h-1 bg-transparent -translate-y-full cursor-ns-resize"
          onPointerDown={onPointerDown}
        ></div>
      </div>
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
          <Button onClick={() => AnimationManager.stop()}>
            <StopIcon />
          </Button>
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
