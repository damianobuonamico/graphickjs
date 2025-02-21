import { Component } from 'solid-js';
import ColorPropertyValue from './ColorPropertyValue';
import ComponentPanel from './ComponentPanel';

interface BackgroundComponentData {
  color: [number, number, number, number];
}

const BackgroundComponent: Component<{
  data: BackgroundComponentData;
  setData: (data: Partial<BackgroundComponentData>) => void;
}> = (props) => {
  return (
    <ComponentPanel title="Background">
      <ColorPropertyValue
        value={props.data.color}
        // onInput={() => {
        // props.component.value.tempSet(val);
        // }}
        onInput={(color) => {
          props.setData({ color });
        }}
      ></ColorPropertyValue>
    </ComponentPanel>
  );
};

export default BackgroundComponent;
