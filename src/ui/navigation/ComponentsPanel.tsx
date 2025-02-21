import { Component, Show } from 'solid-js';
import { BackgroundComponent, ColorPropertyValue } from '../components';

const ComponentsPanel: Component<{
  onResize: (x: number) => void;
  components: ComponentsState;
  setComponents: (components: ComponentsState) => void;
}> = (props) => {
  const onPointerDown = () => {
    window.addEventListener('pointermove', onPointerMove);
    window.addEventListener('pointerup', onPointerUp);
  };

  const onPointerMove = (e: PointerEvent) => props.onResize(e.clientX);

  const onPointerUp = () => {
    window.removeEventListener('pointermove', onPointerMove);
    window.removeEventListener('pointerup', onPointerUp);
  };

  return (
    <div class="bg-primary-800 w-full h-full border-primary-600 border-l flex">
      <div class="w-0 h-full z-[1000]">
        <div
          class="w-1 h-full bg-transparent -translate-x-full cursor-ew-resize"
          onPointerDown={onPointerDown}
        ></div>
      </div>
      <div class="flex w-full h-full flex-col">
        <Show when={props.components.background !== undefined}>
          <BackgroundComponent
            data={{ color: props.components.background! }}
            setData={(data) => props.setComponents({ background: data.color })}
          />
        </Show>
      </div>
    </div>
  );
};

export default ComponentsPanel;
