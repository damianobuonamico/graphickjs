import { Component } from 'solid-js';
import TitleBar from '@navigation/TitleBar';
import { createStore } from 'solid-js/store';
import { Mode, State, Tool } from './types';
import { ToolBar } from '@/ui/navigation';

const Editor: Component = () => {
  const [state, setState] = createStore<State>({
    mode: 'designer',
    tool: 'select'
  });

  return (
    <div class="w-screen h-screen bg-primary-700 grid grid-rows-title-bar">
      <TitleBar
        mode={state.mode}
        setMode={(mode: Mode) => setState({ mode })}
      />
      <div class="grid grid-cols-tool-bar">
        <ToolBar
          tools={['select', 'vselect', 'pen']}
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
