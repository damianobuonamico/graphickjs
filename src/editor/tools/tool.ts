import InputManager from '../input';
import SceneManager from '../scene';
import SelectionManager from '../selection';
import onPanPointerDown from './pan';
import onPenPointerDown, { onPenPointerHover } from './pen';
import onPolygonPointerDown from './polygon';
import onRotatePointerDown from './rotate';
import onScalePointerDown from './scale';
import onSelectPointerDown from './select';
import onDirectSelectPointerDown from './directSelect';
import onZoomPointerDown from './zoom';

class ToolState {
  private m_current: Tool;
  private m_active: Tool;
  private m_last: Tool;

  private m_callbacks: ToolMap<() => PointerDownReturn> = {
    select: onSelectPointerDown,
    directSelect: onDirectSelectPointerDown,
    pen: onPenPointerDown,
    rectangle: () => onPolygonPointerDown('rectangle'),
    ellipse: () => onPolygonPointerDown('ellipse'),
    pan: onPanPointerDown,
    zoom: onZoomPointerDown,
    scale: onScalePointerDown,
    rotate: onRotatePointerDown
  };
  private m_data: ToolMap<ToolData> = {
    select: {},
    directSelect: {},
    pen: {},
    rectangle: {},
    ellipse: {},
    pan: {},
    zoom: {},
    scale: {},
    rotate: {}
  };
  private m_hovers: Partial<ToolMap<() => void>> = {
    pen: onPenPointerHover
  };

  private m_vertexTools = { directSelect: true, pen: true };

  private m_setTool: (tool: Tool) => void;
  private m_onPointerMove: (() => void) | undefined;
  private m_onPointerUp: ((abort: boolean) => void) | undefined;
  private m_onKey: ((e: KeyboardEvent) => void) | undefined;

  constructor(setTool: (tool: Tool) => void, tool: Tool) {
    this.m_setTool = setTool;
    this.current = tool;
  }

  public get current() {
    return this.m_current;
  }

  public set current(tool: Tool) {
    this.m_current = tool;
    this.recalculate();
  }

  public get active() {
    return this.m_active;
  }

  public set active(tool: Tool) {
    this.m_last = this.m_active;
    this.m_active = tool;
    this.m_setTool(tool);

    SceneManager.overlays.clear();
    SceneManager.render();

    this.onPointerHover();

    SelectionManager.calculateRenderOverlay();
  }

  public get isVertex() {
    return this.m_vertexTools.hasOwnProperty(this.active);
  }

  public get data() {
    return this.m_data[this.m_active];
  }

  public recalculate() {
    if (InputManager.keys.space) {
      if (InputManager.keys.ctrl) {
        this.active = 'zoom';
      } else {
        this.active = 'pan';
      }
    } else if (InputManager.keys.ctrl && SceneManager.state.workspace !== 'whiteboard') {
      if (this.m_current === 'select' || this.m_current === 'pen') this.active = 'directSelect';
      else this.active = 'select';
    } else {
      this.active = this.current;
    }
  }

  public abort() {
    this.m_data[this.m_active] = {};
  }

  public onPointerDown(override?: 'scale' | 'rotate') {
    let active = this.m_active;

    if (override && (override === 'scale' || override === 'rotate')) active = override;

    if (this.m_last !== active && active !== 'pan' && active !== 'zoom')
      this.m_data[this.m_last] = {};

    const callbacks = this.m_callbacks[active]();

    this.m_onPointerMove = callbacks.onPointerMove;
    this.m_onPointerUp = callbacks.onPointerUp;
    this.m_onKey = callbacks.onKey;
  }

  public onPointerMove() {
    if (this.m_onPointerMove) this.m_onPointerMove();
  }

  public onPointerHover() {
    if (this.m_hovers[this.m_active]) this.m_hovers[this.m_active]!();
  }

  public onPointerUp(abort: boolean) {
    if (this.m_onPointerUp) this.m_onPointerUp(abort);
  }

  public onKey(e: KeyboardEvent) {
    if (this.m_onKey) {
      this.m_onKey(e);
      SceneManager.render();
    }
  }
}

export default ToolState;
