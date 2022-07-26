import { Component } from 'solid-js';
import TitleBar from '@navigation/TitleBar';
import { createStore } from 'solid-js/store';
import { Mode, State } from './types';

const Editor: Component = () => {
  const [state, setState] = createStore<State>({ mode: 'designer' });

  return (
    <div class="w-screen h-screen bg-primary-700">
      <TitleBar
        mode={state.mode}
        setMode={(mode: Mode) => setState({ mode })}
      />
    </div>
  );
};

export default Editor;
