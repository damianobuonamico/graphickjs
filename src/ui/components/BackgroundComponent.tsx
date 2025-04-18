import { Component } from 'solid-js';
import ComponentPanel from './ComponentPanel';
import { ColorInput } from '../inputs';

const BackgroundComponent: Component<{
  data: BackgroundComponentData;
  setData: (data: Partial<BackgroundComponentData>) => void;
}> = (props) => {
  return (
    <ComponentPanel title="Background">
      <ColorInput value={props.data.color} onInput={(color) => props.setData({ color })} />
    </ComponentPanel>
  );
};

export default BackgroundComponent;
