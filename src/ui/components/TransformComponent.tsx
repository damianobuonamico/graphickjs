import { Component } from 'solid-js';
import ComponentPanel from './ComponentPanel';
import { NumberInput } from '../inputs';
import Checkbox from '../inputs/Checkbox';

const TransformComponent: Component<{
  data: TransformComponentData;
  setData: (data: Operation<TransformComponentData>) => void;
}> = (props) => {
  return (
    <ComponentPanel title="Transform">
      <div class="flex">
        <NumberInput
          type="float"
          onChange={(x) => props.setData({ x })}
          value={props.data.x}
          leftIcon="X"
        />
        <NumberInput
          type="float"
          onChange={(y) => props.setData({ y })}
          value={props.data.y}
          leftIcon="Y"
          class="ml-1.5"
        />
        {/* <Checkbox checked={true} checkedIcon="check" uncheckedIcon="hand" class="ml-1.5" /> */}
      </div>
      <div class="flex mt-1.5">
        <NumberInput
          type="float"
          onChange={(w) => props.setData({ w })}
          value={props.data.w}
          leftIcon="W"
        />
        <NumberInput
          type="float"
          onChange={(h) => props.setData({ h })}
          value={props.data.h}
          leftIcon="H"
          class="ml-1.5"
        />
      </div>
    </ComponentPanel>
  );
};

export default TransformComponent;
