import { Component, createEffect, Match, onMount, Switch } from 'solid-js';
import { createStore } from 'solid-js/store';
import { ToolBar, TitleBar, Timeline } from '@navigation';
import { CanvasDOM } from '@multimedia';
import Renderer from './renderer/renderer';
import SceneManager from './scene';
import InputManager from './input';
import { ComponentsPanel } from '@/ui/components';
import { classNames } from '@/utils/utils';
import Whiteboard from '@/ui/workspaces/Whiteboard';
import Designer from '@/ui/workspaces/Designer';

function getModePrimaryColor(mode: Mode) {
  switch (mode) {
    case 'whiteboard':
      return '#c867e6';
    case 'publisher':
      return '#ffa666';
    default:
      return '#38c3f2';
  }
}

const Editor: Component = () => {
  const [state, setState] = createStore<State>({
    mode: 'whiteboard',
    tool: 'select',
    loading: true,
    timeline: true,
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
    <Switch fallback={<Designer state={state} setState={setState} />}>
      <Match when={state.mode === 'whiteboard'}>
        <Whiteboard state={state} setState={setState} />
      </Match>
    </Switch>
  );
};

export default Editor;
