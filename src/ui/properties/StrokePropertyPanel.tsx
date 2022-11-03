import Color from '@/editor/ecs/components/color';
import SelectionManager from '@/editor/selection';
import { Component, For, Show } from 'solid-js';
import { createStore } from 'solid-js/store';
import ColorPropertyValue from './ColorPropertyValue';
import PropertyPanel from './PropertyPanel';

const StrokePropertyPanel: Component<{}> = (props) => {
  const [stroke, setStroke] = createStore<StrokePropertyData>({
    active: false,
    mixed: false,
    strokes: [new Color('#000000')]
  });

  SelectionManager.setStrokePropertyFn = (data: Partial<StrokePropertyData>) => setStroke(data);

  return (
    <Show when={stroke.active}>
      <PropertyPanel title="Stroke">
        <For each={stroke.strokes}>
          {(item) => (
            <ColorPropertyValue
              value={item}
              onInput={(color: string) =>
                SelectionManager.setStroke({ color, updateUI: false, commit: false })
              }
              onChange={() => SelectionManager.setStroke({ commit: true })}
            ></ColorPropertyValue>
          )}
        </For>
      </PropertyPanel>
    </Show>
  );
};

export default StrokePropertyPanel;
