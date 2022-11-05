import SelectionManager from '@/editor/selection';
import { classNames } from '@/utils/utils';
import { Component } from 'solid-js';
import {
  BorderWidthIcon,
  CornerBevelIcon,
  CornerMiterIcon,
  CornerRoundIcon,
  EyeClosedIcon,
  EyeOpenIcon,
  MinusIcon,
  PlusIcon
} from '../icons';
import { Button, Select } from '../inputs';
import ColorPropertyValue from './ColorPropertyValue';
import PropertyPanel from './PropertyPanel';
import PropertyValue from './PropertyValue';

const StrokeComponentPanel: Component<{ component: StrokeComponentCollection }> = (props) => {
  return (
    <PropertyPanel
      title="Stroke"
      controls={
        props.component.color.mixed ? (
          <Button
            onClick={() =>
              SelectionManager.setComponents(
                {
                  stroke: { ...props.component.color.value, color: props.component.color.value.hex }
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
                    stroke: { visible: !props.component.color.visible }
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
      <div class="mt-1 flex">
        <PropertyValue correctTextPadding={true} rightPadding={false}>
          <div class="flex items-center">
            <BorderWidthIcon />
            <input
              class="ml-2.5 w-9 bg-transparent outline-none"
              type="text"
              value={props.component.width.mixed ? 'Mixed' : props.component.width.value}
              onChange={(e) => {
                SelectionManager.setComponents(
                  {
                    stroke: {
                      width: (e.target as any).value
                    }
                  },
                  {}
                );
                (e.target as any).blur();
              }}
            />
          </div>
        </PropertyValue>
        <PropertyValue class="ml-2" disablePadding={true}>
          <Select
            value={props.component.corner.mixed ? 'null' : props.component.corner.value}
            optionsStyle={{ width: '30px', height: '30px' }}
            options={[
              { id: 'miter', label: <CornerMiterIcon /> },
              { id: 'bevel', label: <CornerBevelIcon /> },
              { id: 'round', label: <CornerRoundIcon /> }
            ]}
            onChange={(id: CanvasLineJoin) =>
              SelectionManager.setComponents(
                {
                  stroke: {
                    corner: id
                  }
                },
                {}
              )
            }
          />
        </PropertyValue>
      </div>
    </PropertyPanel>
  );
};

export default StrokeComponentPanel;
