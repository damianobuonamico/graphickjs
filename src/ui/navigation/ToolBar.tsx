import { Component, For, JSX, Match, Switch } from 'solid-js';
import { Tool } from '@editor/types';
import { Button } from '@inputs';
import { PenIcon, PointerIcon, PointerVertexIcon } from '@icons';

interface ToolData {
  icon: JSX.Element;
}

export function getToolData(tool: Tool): ToolData {
  switch (tool) {
    case 'vselect':
      return {
        icon: <PointerVertexIcon />
      };
    case 'pen':
      return {
        icon: <PenIcon />
      };
    default:
      return {
        icon: <PointerIcon />
      };
  }
}

const ToolBar: Component<{
  tools: Array<Tool | 'separator'>;
  tool: Tool;
  setTool(tool: Tool): void;
}> = (props) => {
  return (
    <div class="bg-primary-800 w-10 h-full flex items-center border-primary-600 border-r flex-col">
      <For each={props.tools}>
        {(tool) =>
          tool === 'separator' ? (
            <div class="w-6 h-[1px] bg-primary-600" />
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
