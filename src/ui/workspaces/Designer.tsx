import InputManager from '@/editor/input';
import { classNames } from '@/utils/utils';
import { Component } from 'solid-js';
import { ComponentsPanel } from '../components';
import { CanvasDOM } from '../multimedia';
import { Timeline, TitleBar, ToolBar } from '../navigation';

const Designer: Component<{ state: State; setState: (state: Partial<State>) => void }> = ({
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
      <div class="grid grid-cols-designer">
        <ToolBar
          tools={[
            'select',
            'directSelect',
            'separator',
            'pen',
            'separator',
            ['rectangle', 'ellipse'],
            'separator',
            'pan',
            'zoom'
          ]}
          tool={state.tool}
          setTool={(tool: Tool) => {
            setState({ tool });
            InputManager.tool.current = tool;
          }}
        />
        <div
          class={classNames('grow overflow-hidden z-0 grid')}
          style={{
            'grid-template-rows': state.timeline ? `1fr ${state.timelineHeight}px` : '1fr 0px'
          }}
        >
          <CanvasDOM />
          <Timeline
            onResize={(y) => {
              let height = Math.min(window.innerHeight - y, window.innerHeight - 200);
              if (height < 30) setState({ timeline: false });
              else setState({ timeline: true, timelineHeight: height });

              InputManager.onResize({} as UIEvent);
            }}
          />
        </div>
        <ComponentsPanel />
      </div>
    </div>
  );
};

export default Designer;
