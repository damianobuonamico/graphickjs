type Workspace = 'designer' | 'publisher' | 'whiteboard';

interface ComponentsState {
  background?: BackgroundComponentData;
  fill?: Mixed<FillComponentData>;
  stroke?: Mixed<StrokeComponentData>;
}

interface ComponentsStateOperation {
  background?: Operation<BackgroundComponentData>;
  fill?: Operation<FillComponentData>;
  stroke?: Operation<StrokeComponentData>;
}

interface UIData {
  components?: ComponentsState;
}

interface State {
  name: string;
  workspace: Workspace;
  tool: Tool;
  loading: boolean;
  timeline: boolean;
  timelineHeight: number;
  componentsPanelWidth: number;
  ui_data: UIData;
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
