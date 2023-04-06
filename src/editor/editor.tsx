import { Component, createEffect, Match, onMount, Switch } from "solid-js";
import { createStore } from "solid-js/store";
import Renderer from "./renderer/renderer";
import SceneManager from "./scene";
import InputManager from "./input";
import Whiteboard from "@/ui/workspaces/Whiteboard";
import Designer from "@/ui/workspaces/Designer";
import { getWorkspacePrimaryColor } from "@/utils/color";

const Editor: Component = () => {
  const [state, setState] = createStore<State>({
    name: "Untitled",
    workspace: "whiteboard",
    tool: "select",
    loading: true,
    timeline: false,
    timelineHeight: 500,
  });

  // Renderer.init();
  InputManager.init({}, (tool: Tool) => {
    setState({ tool });
  });
  SceneManager.init(
    state,
    (loading) => setState({ loading }),
    (workspace) => setState({ workspace: workspace })
  );

  createEffect(() => {
    SceneManager.onWorkspaceChange(state.workspace);
    document.documentElement.style.setProperty(
      "--primary-color",
      getWorkspacePrimaryColor(state.workspace)
    );
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
      <Match when={state.workspace === "whiteboard"}>
        <Whiteboard state={state} setState={setState} />
      </Match>
    </Switch>
  );
};

export default Editor;
