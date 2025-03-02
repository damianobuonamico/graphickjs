import { Component, createEffect, For, JSX } from 'solid-js';
import { Button } from '@inputs';
import {
  ArcIcon,
  ArrowIcon,
  CircleIcon,
  EraserIcon,
  EyeDropperIcon,
  GradientIcon,
  HandIcon,
  LineIcon,
  PencilIcon,
  PenIcon,
  PointerIcon,
  PointerNodeGroupIcon,
  PointerNodeIcon,
  PolygonIcon,
  RectangleIcon,
  RoundedRectangleIcon,
  RulerIcon,
  SpiralIcon,
  SplineIcon,
  StarIcon,
  TypeIcon,
  ZoomIcon
} from '@icons';
import MenuSelect from '../menu/MenuSelect';

export function getToolIcon(tool: Tool): JSX.Element {
  switch (tool) {
    case 'selectNode':
      return <PointerNodeIcon />;
    case 'selectNodeGroup':
      return <PointerNodeGroupIcon />;
    case 'pen':
      return <PenIcon />;
    case 'spline':
      return <SplineIcon />;
    case 'text':
      return <TypeIcon />;
    case 'line':
      return <LineIcon />;
    case 'arrow':
      return <ArrowIcon />;
    case 'arc':
      return <ArcIcon />;
    case 'spiral':
      return <SpiralIcon />;
    case 'rectangle':
      return <RectangleIcon />;
    case 'roundedRectangle':
      return <RoundedRectangleIcon />;
    case 'ellipse':
      return <CircleIcon />;
    case 'polygon':
      return <PolygonIcon />;
    case 'star':
      return <StarIcon />;
    case 'gradient':
      return <GradientIcon />;
    case 'eyedropper':
      return <EyeDropperIcon />;
    case 'ruler':
      return <RulerIcon />;
    case 'pan':
      return <HandIcon />;
    case 'zoom':
      return <ZoomIcon />;
    case 'pencil':
      return <PencilIcon />;
    case 'eraser':
      return <EraserIcon />;
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
            <MenuSelect
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
