import { Component, Show } from 'solid-js';
import { BackgroundComponent, FillComponent, StrokeComponent } from '../components';

const ComponentsPanel: Component<{
  onResize: (x: number) => void;
  components?: ComponentsState;
  setComponents: (components: ComponentsStateOperation) => void;
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
        <Show when={props.components !== undefined && props.components.background !== undefined}>
          <BackgroundComponent
            data={props.components!.background!}
            setData={(data) => props.setComponents({ background: data })}
          />
        </Show>
        <Show when={props.components !== undefined && props.components.fill !== undefined}>
          <FillComponent
            data={props.components!.fill!}
            setData={(data) => props.setComponents({ fill: data })}
          />
        </Show>
        <Show when={props.components !== undefined && props.components.stroke !== undefined}>
          <StrokeComponent
            data={props.components!.stroke!}
            setData={(data) => props.setComponents({ stroke: data })}
          />
        </Show>
      </div>
    </div>
  );
};

export default ComponentsPanel;
