import { Component, Show } from 'solid-js';
import ComponentPanel from './ComponentPanel';
import { ColorInput } from '../inputs';
import Checkbox from '../inputs/Checkbox';

const FillComponent: Component<{
  data: Mixed<FillComponentData>;
  setData: (data: Operation<FillComponentData>) => void;
}> = (props) => {
  return (
    <ComponentPanel title="Fill">
      <div class="flex ">
        <ColorInput
          value={props.data.color}
          onInput={(color) => {
            props.setData({ color });
          }}
        />
        <Checkbox
          checked={props.data.visible}
          checkedIcon="eyeOpen"
          uncheckedIcon="eyeClosed"
          class="ml-1.5"
          onChange={(visible: boolean) => props.setData({ visible })}
        />
      </div>
    </ComponentPanel>
  );
};

export default FillComponent;
