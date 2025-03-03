import { Component } from 'solid-js';
import ComponentPanel from './ComponentPanel';
import { Button, NumberInput } from '../inputs';
import Checkbox from '../inputs/Checkbox';

const TransformComponent: Component<{
  data: TransformComponentData;
  setData: (data: Operation<TransformComponentData>) => void;
  refreshUI: () => void;
}> = (props) => {
  return (
    <ComponentPanel title="Transform" class="flex flex-row">
      <div class="flex flex-col w-full">
        <div class="flex">
          <NumberInput
            type="float"
            onChange={(x) => props.setData({ x })}
            onSubmit={() => props.refreshUI()}
            value={props.data.x}
            leftIcon="X"
          />
          <NumberInput
            type="float"
            onChange={(y) => props.setData({ y })}
            onSubmit={() => props.refreshUI()}
            value={props.data.y}
            leftIcon="Y"
            class="ml-1.5"
          />
        </div>
        <div class="flex mt-1.5">
          <NumberInput
            type="float"
            onChange={(w) => props.setData({ w })}
            onSubmit={() => props.refreshUI()}
            value={props.data.w}
            leftIcon="W"
            min={0}
          />
          <NumberInput
            type="float"
            onChange={(h) => props.setData({ h })}
            onSubmit={() => props.refreshUI()}
            value={props.data.h}
            leftIcon="H"
            class="ml-1.5"
            min={0}
          />
        </div>
        <div class="flex mt-1.5">
          <NumberInput
            type="float"
            onChange={(angle) => props.setData({ angle })}
            onSubmit={() => props.refreshUI()}
            value={props.data.angle}
            leftIcon="angle"
            class="w-full"
          />
          <div class="flex ml-1.5 w-full justify-between">
            <Button variant="small-button">O</Button>
            <Button variant="small-button">O</Button>
            <Button variant="small-button">O</Button>
          </div>
        </div>
      </div>
      <div class="flex ml-1.5 items-center">
        <Checkbox checked={true} checkedIcon="link" uncheckedIcon="linkNone" />
      </div>
    </ComponentPanel>
  );
};

export default TransformComponent;
