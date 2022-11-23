import InputManager from '@/editor/input';
import { classNames } from '@/utils/utils';
import { Component } from 'solid-js';
import { CanvasDOM } from '../multimedia';
import { TitleBar, ToolBar } from '../navigation';

const Whiteboard: Component<{ state: State; setState: (state: Partial<State>) => void }> = ({
  state,
  setState
}) => {
  return (
    <div class="w-screen h-screen bg-primary-700 grid grid-rows-title-bar">
      <TitleBar
        mode={state.mode}
        setMode={(mode: Workspace) => setState({ mode })}
        loading={state.loading}
      />
      <div class="grid grid-cols-whiteboard">
        <ToolBar
          tools={['select', 'separator', ['rectangle', 'ellipse'], 'separator', 'pan', 'zoom']}
          tool={state.tool}
          setTool={(tool: Tool) => {
            setState({ tool });
            InputManager.tool.current = tool;
          }}
        />
        <div class={classNames('grow overflow-hidden z-0 grid')}>
          <CanvasDOM />
        </div>
      </div>
    </div>
  );
};

export default Whiteboard;
