import { Component, createEffect } from 'solid-js';
import { createStore } from 'solid-js/store';
import { ToolBar, TitleBar } from '@navigation';
import { CanvasDOM } from '@multimedia';
import { Canvas2D, CanvasGl } from '@renderer';

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
  const useWebGL = true;
  const canvas: Canvas = useWebGL ? new CanvasGl() : new Canvas2D();

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
        <CanvasDOM canvas={canvas} />
      </div>
    </div>
  );
};

export default Editor;
