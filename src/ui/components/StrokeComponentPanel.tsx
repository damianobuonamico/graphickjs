import SelectionManager from '@/editor/selection';
import { classNames } from '@/utils/utils';
import { Component } from 'solid-js';
import { EyeClosedIcon, EyeOpenIcon, MinusIcon, PlusIcon } from '../icons';
import { Button } from '../inputs';
import ColorPropertyValue from './ColorPropertyValue';
import PropertyPanel from './PropertyPanel';
import PropertyValue from './PropertyValue';

const StrokeComponentPanel: Component<{ component: GlobalComponent<StrokeComponent> }> = (
  props
) => {
  return (
    <PropertyPanel
      title="Stroke"
      controls={
        props.component.mixed ? (
          <Button
            onClick={() =>
              SelectionManager.setComponents(
                { stroke: { ...props.component.value, color: props.component.value.color.hex } },
                { replace: true }
              )
            }
          >
            <PlusIcon />
          </Button>
        ) : undefined
      }
    >
      {props.component.mixed ? (
        <PropertyValue hoverEffect={false} class="text-primary-300">
          Click + to replace mixed content
        </PropertyValue>
      ) : (
        <div
          class={classNames('flex justify-between', {
            'text-primary-400': !props.component.value.visible
          })}
        >
          <ColorPropertyValue
            value={props.component.value.color}
            onInput={(value, format) =>
              SelectionManager.setComponents(
                {
                  stroke: { color: value, format }
                },
                { commit: false }
              )
            }
            onChange={() => SelectionManager.setComponents({ stroke: {} }, { commit: true })}
          ></ColorPropertyValue>
          <div class="flex">
            <Button
              onClick={() =>
                SelectionManager.setComponents(
                  {
                    stroke: { visible: !props.component.value.visible }
                  },
                  {}
                )
              }
            >
              {props.component.value.visible ? <EyeOpenIcon /> : <EyeClosedIcon />}
            </Button>
            <Button>
              <MinusIcon />
            </Button>
          </div>
        </div>
      )}
    </PropertyPanel>
  );
};

export default StrokeComponentPanel;
