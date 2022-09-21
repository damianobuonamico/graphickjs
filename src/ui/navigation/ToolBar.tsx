import { Component, createEffect, For, JSX } from 'solid-js';
import { Button } from '@inputs';
import {
  CircleIcon,
  HandIcon,
  PenIcon,
  PointerIcon,
  PointerVertexIcon,
  RectangleIcon,
  ZoomIcon
} from '@icons';
import Select from '../inputs/Select';

export function getToolIcon(tool: Tool): JSX.Element {
  switch (tool) {
    case 'vselect':
      return <PointerVertexIcon />;
    case 'pen':
      return <PenIcon />;
    case 'rectangle':
      return <RectangleIcon />;
    case 'ellipse':
      return <CircleIcon />;
    case 'pan':
      return <HandIcon />;
    case 'zoom':
      return <ZoomIcon />;
    default:
      return <PointerIcon />;
  }
}

const ToolBar: Component<{
  tools: Array<Tool | Tool[] | 'separator'>;
  tool: Tool;
  setTool(tool: Tool): void;
}> = (props) => {
  return (
    <div class="bg-primary-800 w-10 h-full flex items-center border-primary-600 border-r flex-col">
      <For each={props.tools}>
        {(tool) =>
          tool === 'separator' ? (
            <div class="w-6 h-[1px] bg-primary-600" />
          ) : Array.isArray(tool) ? (
            <Select
              menuButton={{ variant: 'tool', arrow: true }}
              options={tool.map((tool) => {
                return { id: tool, label: getToolIcon(tool) };
              })}
              position={'right'}
              onClick={(current: string) => props.setTool(current as Tool)}
              onChange={(current: string) => props.setTool(current as Tool)}
              useLongPress={true}
              active={tool.includes(props.tool)}
            />
          ) : (
            <Button
              active={tool === props.tool}
              variant={'tool'}
              onClick={() => props.setTool(tool)}
            >
              {getToolIcon(tool)}
            </Button>
          )
        }
      </For>
    </div>
  );
};

export default ToolBar;
