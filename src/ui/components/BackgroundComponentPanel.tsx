import SceneManager from '@/editor/scene';
import { Component } from 'solid-js';
import ColorPropertyValue from './ColorPropertyValue';
import PropertyPanel from './PropertyPanel';

const BackgroundComponentValue: Component<{ component: GlobalComponent<ColorComponent> }> = (
  props
) => {
  return (
    <PropertyPanel title="Background">
      <ColorPropertyValue
        value={props.component.value}
        onInput={(val) => {
          props.component.value.tempSet(val);
          SceneManager.render();
        }}
        onChange={() => {
          props.component.value.apply();
        }}
      ></ColorPropertyValue>
    </PropertyPanel>
  );
};

export default BackgroundComponentValue;
