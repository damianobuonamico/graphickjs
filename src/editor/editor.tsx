import { Component, createEffect, onMount, Show } from 'solid-js';
import { createStore } from 'solid-js/store';
import { ToolBar, TitleBar, Timeline } from '@navigation';
import { CanvasDOM } from '@multimedia';
import Renderer from './renderer/renderer';
import SceneManager from './scene';
import InputManager from './input';
import { ComponentsPanel } from '@/ui/components';
import { classNames } from '@/utils/utils';

function getModePrimaryColor(mode: Mode) {
  switch (mode) {
    case 'photo':
      return '#c867e6';
    case 'publisher':
      return '#ffa666';
    default:
      return '#38c3f2';
  }
}

const Editor: Component = () => {
  const [state, setState] = createStore<State>({
    mode: 'designer',
    tool: 'select',
    loading: true,
    timeline: false,
    timelineHeight: 500
  });
  Renderer.init();
  SceneManager.init((loading) => {
    setState({ loading });
  });

  createEffect(() => {
    document.documentElement.style.setProperty('--primary-color', getModePrimaryColor(state.mode));
  });

  onMount(() =>
    setTimeout(() => {
      InputManager.init({}, (tool: Tool) => {
        setState({ tool });
      });
      SceneManager.render();
    }, 25)
  );

  return (
    <div class="w-screen h-screen bg-primary-700 grid grid-rows-title-bar">
      <TitleBar
        mode={state.mode}
        setMode={(mode: Mode) => setState({ mode })}
        loading={state.loading}
      />
      <div class="grid grid-cols-editor">
        <ToolBar
          tools={[
            'select',
            'vselect',
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

export default Editor;
