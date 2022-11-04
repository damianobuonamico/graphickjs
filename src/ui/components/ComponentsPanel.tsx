import SelectionManager from '@/editor/selection';
import { Component, Show } from 'solid-js';
import { createStore } from 'solid-js/store';
import BackgroundComponentValue from './BackgroundComponentPanel';
import FillComponentPanel from './FillComponentPanel';
import StrokeComponentPanel from './StrokeComponentPanel';

const ComponentsPanel: Component<{}> = (props) => {
  const [components, setComponents] = createStore<ComponentCollection>({});
  SelectionManager.updateComponentsFn = setComponents;

  return (
    <div class="bg-primary-800 h-full w-full border-primary-600 border-l">
      <Show when={components.background}>
        <BackgroundComponentValue component={components.background!} />
      </Show>
      <Show when={components.fill}>
        <FillComponentPanel component={components.fill!} />
      </Show>
      <Show when={components.stroke}>
        <StrokeComponentPanel component={components.stroke!} />
      </Show>
    </div>
  );
};

export default ComponentsPanel;
