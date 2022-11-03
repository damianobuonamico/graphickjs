import Color from '@/editor/ecs/components/color';
import SelectionManager from '@/editor/selection';
import { Component, For, Show } from 'solid-js';
import { createStore } from 'solid-js/store';
import ColorPropertyValue from './ColorPropertyValue';
import PropertyPanel from './PropertyPanel';

const FillPropertyPanel: Component<{}> = (props) => {
  const [fill, setFill] = createStore<FillPropertyData>({
    active: false,
    mixed: false,
    fills: [new Color('#FFFFFF')]
  });

  SelectionManager.setFillPropertyFn = (data: Partial<FillPropertyData>) => setFill(data);

  return (
    <Show when={fill.active}>
      <PropertyPanel title="Fill">
        <For each={fill.fills}>
          {(item) => (
            <ColorPropertyValue
              value={item}
              onInput={(color: string) =>
                SelectionManager.setFill({ color, updateUI: false, commit: false })
              }
              onChange={() => SelectionManager.setFill({ commit: true })}
            ></ColorPropertyValue>
          )}
        </For>
      </PropertyPanel>
    </Show>
  );
};

export default FillPropertyPanel;
