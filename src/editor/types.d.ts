type Workspace = 'designer' | 'publisher' | 'whiteboard';

interface ComponentsState {
  background?: [number, number, number, number];
}

interface State {
  name: string;
  workspace: Workspace;
  tool: Tool;
  loading: boolean;
  timeline: boolean;
  timelineHeight: number;
  componentsPanelWidth: number;
  components: ComponentsState;
}

interface MountedListener {
  type: keyof HTMLElementEventMap | keyof WindowEventMap;
  callback(e: Event): void;
  target: HTMLElement | Window | Document;
  options?: boolean | AddEventListenerOptions;
}

interface TouchID {
  position: vec2;
  prev: vec2;
  to: vec2;
  id: number;
}
