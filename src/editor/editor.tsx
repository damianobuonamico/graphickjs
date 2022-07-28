import { Component, createEffect } from 'solid-js';
import TitleBar from '@navigation/TitleBar';
import { createStore } from 'solid-js/store';
import { Mode, State, Tool } from './types';
import { ToolBar } from '@/ui/navigation';

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
    tool: 'select'
  });

  createEffect(() => {
    document.documentElement.style.setProperty(
      '--primary-color',
      getModePrimaryColor(state.mode)
    );
  });

  return (
    <div class="w-screen h-screen bg-primary-700 grid grid-rows-title-bar">
      <TitleBar
        mode={state.mode}
        setMode={(mode: Mode) => setState({ mode })}
      />
      <div class="grid grid-cols-tool-bar">
        <ToolBar
          tools={[
            'select',
            'vselect',
            'separator',
            'pen',
            'separator',
            ['rectangle', 'ellipse']
          ]}
          tool={state.tool}
          setTool={(tool: Tool) => {
            setState({ tool });
          }}
        />
      </div>
    </div>
  );
};

export default Editor;
