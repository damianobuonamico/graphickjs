import { Component } from 'solid-js';
import ComponentPanel from './ComponentPanel';
import { Button, Select, ColorInput, NumberInput } from '../inputs';
import Checkbox from '../inputs/Checkbox';
import MixerHorizontal from '../icons/MixerHorizontal';
import { Popover } from '../overlays';
import Toggle from '../inputs/Toggle';
import getIcon from '../icons';

const StrokeComponent: Component<{
  data: Mixed<StrokeComponentData>;
  setData: (data: Operation<StrokeComponentData>) => void;
}> = (props) => {
  return (
    <ComponentPanel title="Stroke">
      <div class="flex">
        <ColorInput
          value={props.data.color as vec4}
          onInput={(color) => props.setData({ color })}
        />
        <Checkbox
          checked={props.data.visible}
          checkedIcon="eyeOpen"
          uncheckedIcon="eyeClosed"
          class="ml-1.5"
          onChange={(visible: boolean) => props.setData({ visible })}
        />
      </div>
      <div class="flex mt-1.5">
        <Select
          options={[
            { id: 'solid', label: 'Solid', icon: 'borderSolid' },
            { id: 'dashed', label: 'Dashed', icon: 'borderDashed' },
            { id: 'dotted', label: 'Dotted', icon: 'borderDotted' }
          ]}
          value={'solid'}
          onChange={(val) => {}}
        />
        <NumberInput
          value={props.data.width}
          leftIcon="borderWidth"
          type="float"
          class="ml-1.5"
          onChange={(width: number) => props.setData({ width })}
        />
        <Popover>
          <Button variant="small-button" class="ml-1.5">
            <MixerHorizontal />
          </Button>

          <div>
            <div class="w-full p-3 px-4 border-b border-primary-600">
              <a class="font-semibold">Stroke settings</a>
            </div>
            <div class="flex">
              <div class="w-full flex flex-col p-2 pl-4 text-primary-200">
                <div class="h-input items-center flex">
                  <a>Cap</a>
                </div>
                <div class="h-input items-center flex mt-1.5">
                  <a>Join</a>
                </div>
              </div>
              <div class="w-full flex flex-col p-2 pl-4">
                <div class="w-full flex items-center">
                  <Toggle
                    options={[
                      { id: 0, label: getIcon('capButt') },
                      { id: 1, label: getIcon('capRound') },
                      { id: 2, label: getIcon('capSquare') }
                    ]}
                    value={props.data.cap}
                    onChange={(cap: number) => props.setData({ cap })}
                  />
                </div>
                <div class="w-full flex items-center mt-1.5">
                  <Toggle
                    options={[
                      { id: 0, label: getIcon('joinMiter') },
                      { id: 1, label: getIcon('joinRound') },
                      { id: 2, label: getIcon('joinBevel') }
                    ]}
                    value={props.data.join}
                    onChange={(join: number) => props.setData({ join })}
                  />
                </div>
              </div>
            </div>
          </div>
        </Popover>
      </div>
    </ComponentPanel>
  );
};

export default StrokeComponent;
