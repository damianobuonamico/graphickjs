import { Component, createEffect, Match, onMount, Switch } from 'solid-js';
import { createStore } from 'solid-js/store';
import InputManager from './input';
import API from '@/wasm/loader';
import Whiteboard from '@/ui/workspaces/Whiteboard';
import Designer from '@/ui/workspaces/Designer';
import { getWorkspacePrimaryColor } from '@/utils/color';

const Editor: Component = () => {
  const [state, setState] = createStore<State>({
    name: 'Untitled',
    workspace: 'designer',
    tool: 'select',
    loading: false,
    timeline: false,
    timelineHeight: 500,
    componentsPanelWidth: 250,
    components: {}
  });

  InputManager.init((tool: Tool) => {
    setState({ tool });
  });

  createEffect(() => {
    document.documentElement.style.setProperty(
      '--primary-color',
      getWorkspacePrimaryColor(state.workspace)
    );
  });

  const onMessage = (msgID: number) => {
    switch (msgID) {
      case 0:
        setState({ components: API._ui_data() });
        break;
    }
  };

  onMount(() => {
    (window as any)['msgbus'] = { send: onMessage };

    setTimeout(() => {
      InputManager.init((tool: Tool) => {
        setState({ tool });
      });
    }, 25);
  });

  return (
    <Switch fallback={<Designer state={state} setState={setState} />}>
      <Match when={state.workspace === 'whiteboard'}>
        <Whiteboard state={state} setState={setState} />
      </Match>
    </Switch>
  );
};

export default Editor;
