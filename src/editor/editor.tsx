import { Component, createEffect, onMount } from 'solid-js';
import { createStore } from 'solid-js/store';
import { ToolBar, TitleBar } from '@navigation';
import { CanvasDOM } from '@multimedia';
import Renderer from './renderer/renderer';
import SceneManager from './scene';
import InputManager from './input';

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
  const useWebGL = false;
  Renderer.init(useWebGL);
  SceneManager.init();

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
      <TitleBar mode={state.mode} setMode={(mode: Mode) => setState({ mode })} />
      <div class="grid grid-cols-tool-bar">
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
            InputManager.tool = state.tool;
          }}
        />
        <CanvasDOM />
      </div>
    </div>
  );
};

export default Editor;
