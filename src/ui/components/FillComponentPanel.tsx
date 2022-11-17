import SelectionManager from '@/editor/selection';
import { classNames } from '@/utils/utils';
import { Component, createEffect } from 'solid-js';
import { EyeClosedIcon, EyeOpenIcon, MinusIcon, PlusIcon } from '../icons';
import { Button } from '../inputs';
import ColorPropertyValue from './ColorPropertyValue';
import PropertyPanel from './PropertyPanel';
import PropertyValue from './PropertyValue';

const FillPropertyPanel: Component<{ component: FillComponentCollection }> = (props) => {
  return (
    <PropertyPanel
      title="Fill"
      controls={
        props.component.color.mixed ? (
          <Button
            onClick={() =>
              SelectionManager.setComponents(
                {
                  fill: {
                    ...props.component,
                    color: props.component.color.value.hex,
                    visible: true
                  }
                },
                { replace: true }
              )
            }
          >
            <PlusIcon />
          </Button>
        ) : undefined
      }
    >
      {props.component.color.mixed ? (
        <PropertyValue hoverEffect={false} class="text-primary-300">
          Click + to replace mixed content
        </PropertyValue>
      ) : (
        <div
          class={classNames('flex justify-between', {
            'text-primary-400': !props.component.color.visible
          })}
        >
          <ColorPropertyValue
            value={props.component.color.value}
            onInput={(value, format) =>
              SelectionManager.setComponents(
                {
                  fill: { color: value, format }
                },
                { commit: false }
              )
            }
          ></ColorPropertyValue>
          <div class="flex">
            <Button
              onClick={() =>
                SelectionManager.setComponents(
                  {
                    fill: { visible: !props.component.color.visible }
                  },
                  {}
                )
              }
            >
              {props.component.color.visible ? <EyeOpenIcon /> : <EyeClosedIcon />}
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

export default FillPropertyPanel;
