import { Component, For, JSX } from 'solid-js';
import { Button } from '@inputs';
import {
  CircleIcon,
  PenIcon,
  PointerIcon,
  PointerVertexIcon,
  RectangleIcon
} from '@icons';
import Select from '../inputs/Select';

interface ToolData {
  icon: JSX.Element;
}

export function getToolData(tool: Tool): ToolData {
  switch (tool) {
    case 'vselect':
      return { icon: <PointerVertexIcon /> };
    case 'pen':
      return { icon: <PenIcon /> };
    case 'rectangle':
      return { icon: <RectangleIcon /> };
    case 'ellipse':
      return { icon: <CircleIcon /> };
    default:
      return { icon: <PointerIcon /> };
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
                return { id: tool, label: getToolData(tool).icon };
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
              {getToolData(tool).icon}
            </Button>
          )
        }
      </For>
    </div>
  );
};

export default ToolBar;
