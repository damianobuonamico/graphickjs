import { Component, createEffect, Match, onMount, Switch } from "solid-js";
import { createStore } from "solid-js/store";
import InputManager from "./input";
import Whiteboard from "@/ui/workspaces/Whiteboard";
import Designer from "@/ui/workspaces/Designer";
import { getWorkspacePrimaryColor } from "@/utils/color";

const Editor: Component = () => {
  const [state, setState] = createStore<State>({
    name: "Untitled",
    workspace: "designer",
    tool: "select",
    loading: false,
    timeline: false,
    timelineHeight: 500,
  });

  InputManager.init((tool: Tool) => {
    setState({ tool });
  });

  createEffect(() => {
    document.documentElement.style.setProperty(
      "--primary-color",
      getWorkspacePrimaryColor(state.workspace)
    );
  });

  onMount(() =>
    setTimeout(() => {
      InputManager.init((tool: Tool) => {
        setState({ tool });
      });
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
