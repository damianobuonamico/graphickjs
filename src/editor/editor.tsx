import { Component, createEffect, Match, onMount, Switch } from 'solid-js';
import { createStore } from 'solid-js/store';
import Renderer from './renderer/renderer';
import SceneManager from './scene';
import InputManager from './input';
import Whiteboard from '@/ui/workspaces/Whiteboard';
import Designer from '@/ui/workspaces/Designer';

function getModePrimaryColor(mode: Workspace) {
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
  SceneManager.init(state, (loading) => {
    setState({ loading });
  });

  createEffect(() => {
    SceneManager.onWorkspaceChange(state.mode);
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
