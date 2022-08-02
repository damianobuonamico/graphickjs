import { INPUT_MOVEMENT_THRESHOLD, INPUT_MOVEMENT_THRESHOLD_MULTIPLIER } from '@utils/constants';
import { BUTTONS, KEYS } from '@utils/keys';
import { fillObject, isInputLike } from '@utils/utils';
import { vec2 } from '@math';
import { Renderer } from './renderer';
import SceneManager from './scene';
import { getToolData } from './tools';

abstract class InputManager {
  public static client: PointerCoord;
  public static scene: PointerCoord;

  public static down: boolean = false;
  public static inside: boolean = true;

  public static button: number = 0;
  public static keys: KeysState;

  private static m_moving: boolean = false;
  private static m_abort: boolean = false;
  private static m_shouldResetTool: boolean = false;

  private static m_type: 'touch' | 'pen' | 'mouse' = 'mouse';
  private static m_tool: ToolState;
  private static m_hover: Entity | undefined;

  private static m_listeners: Listeners;
  private static m_mountedListeners: MountedListener[] = [];

  private static m_setTool: (tool: Tool) => void;

  public static init(listeners: Partial<Listeners>, setTool: (tool: Tool) => void) {
    this.m_listeners = fillObject<Listeners>(listeners, {
      copy: () => {},
      paste: () => {},
      cut: () => {},
      keydown: () => {},
      keyup: () => {},
      resize: () => {},
      unload: () => {},
      pointerdown: () => {},
      pointermove: () => {},
      pointerup: () => {},
      pointercancel: () => {},
      pointerleave: () => {},
      pointerenter: () => {},
      wheel: () => {}
    });

    this.m_setTool = setTool;

    this.client = fillObject(
      {},
      {
        position: vec2.create(),
        movement: vec2.create(),
        delta: vec2.create(),
        origin: vec2.create()
      }
    );

    this.scene = fillObject(
      {},
      {
        position: vec2.create(),
        movement: vec2.create(),
        delta: vec2.create(),
        origin: vec2.create()
      }
    );

    this.keys = fillObject(
      {},
      {
        ctrl: false,
        shift: false,
        alt: false,
        space: false
      }
    );

    this.m_tool = fillObject({}, { current: 'select' });
    this.activeTool = 'select';

    this.mount();
  }

  private static addListener<K extends keyof HTMLElementEventMap, T extends keyof WindowEventMap>(
    type: K | T,
    callback: (this: Window, e: any) => any,
    target: HTMLElement | Window | Document = window,
    options?: boolean | AddEventListenerOptions
  ) {
    target.addEventListener(type, callback, options);
    this.m_mountedListeners.push({ type, callback, target, options });
  }

  private static mount() {
    if (this.m_mountedListeners.length) return;

    this.addListener('cut', this.onCut.bind(this));
    this.addListener('copy', this.onCopy.bind(this));
    this.addListener('paste', this.onPaste.bind(this));

    this.addListener('keydown', this.onKeyDown.bind(this));
    this.addListener('keyup', this.onKeyUp.bind(this));

    this.addListener('pointerdown', this.calculateDeviceType.bind(this));

    if (this.m_type === 'touch') {
      this.addListener('touchstart', this.onTouchStart.bind(this), Renderer.canvas);
      this.addListener('touchmove', this.onTouchMove.bind(this));
      this.addListener('touchend', this.onTouchEnd.bind(this));
      this.addListener('touchcancel', this.onTouchEnd.bind(this));
    } else {
      this.addListener('pointerdown', this.onPointerDown.bind(this), Renderer.canvas);
      this.addListener('pointermove', this.onPointerMove.bind(this));
      this.addListener('pointerup', this.onPointerUp.bind(this));
      this.addListener('pointercancel', this.onPointerUp.bind(this));
      this.addListener('pointerenter', this.onPointerEnter.bind(this), document);
      this.addListener('pointerleave', this.onPointerLeave.bind(this), document);
    }

    this.addListener('resize', this.onResize.bind(this));
    this.addListener('contextmenu', (e: UIEvent) => e.preventDefault(), Renderer.canvas);
    this.addListener('unload', this.onUnload.bind(this));
    this.addListener('wheel', this.onWheel.bind(this), window, { passive: false });
  }

  private static unmount() {
    this.m_mountedListeners.forEach((listener) => {
      listener.target.removeEventListener(listener.type, listener.callback);
    });
    this.m_mountedListeners.length = 0;
  }

  public static set tool(tool: Tool) {
    this.m_tool.current = tool;
    this.calculateTool();
  }

  public static set activeTool(tool: Tool) {
    this.m_tool.active = tool;
    const data = getToolData(tool);
    this.m_tool.onPointerDown = data.callback;
    this.m_setTool(tool);
  }

  private static calculateTool() {
    if (this.keys.space) {
      if (this.keys.ctrl) {
        this.activeTool = 'zoom';
      } else {
        this.activeTool = 'pan';
      }
    } else if (this.keys.ctrl) {
      if (this.m_tool.current === 'select' || this.m_tool.current === 'pen')
        this.activeTool = 'vselect';
      else this.activeTool = 'select';
    } else {
      this.activeTool = this.m_tool.current;
    }
  }

  private static setKey(e: PointerEvent | KeyboardEvent) {
    this.keys.altStateChanged = this.keys.alt !== e.altKey;
    this.keys.alt = e.altKey;

    this.keys.ctrlStateChanged = this.keys.ctrl !== (e as any)[KEYS.CTRL];
    this.keys.ctrl = (e as any)[KEYS.CTRL];

    this.keys.shiftStateChanged = this.keys.shift !== e.shiftKey;
    this.keys.shift = e.shiftKey;
  }

  private static setPointer(e: PointerEvent) {
    this.button = e.button;
    this.setKey(e);

    if (this.down) return;
  }

  private static onCut(e: ClipboardEvent) {
    this.m_listeners.cut(e);
  }

  private static onCopy(e: ClipboardEvent) {
    this.m_listeners.copy(e);
  }

  private static onPaste(e: ClipboardEvent) {
    this.m_listeners.paste(e);
  }

  //* Keyboard Events
  private static onKey(e: KeyboardEvent, up = false) {
    this.setKey(e);

    if (isInputLike(e.target)) return;

    if (e.key === KEYS.ESCAPE) {
      // this.pen = null;
      this.m_abort = true;
      // this.unlockCursor();
      // this._api.ops.render();
    } else if (e.key === KEYS.SPACEBAR) {
      this.keys.spaceStateChanged = this.keys.space === up;
      this.keys.space = !up;
    }

    if (this.down) {
      this.m_shouldResetTool = false;
      return;
    }

    if (this.keys.ctrlStateChanged || this.keys.spaceStateChanged) {
      this.calculateTool();
    }

    // if (this.__alt) {
    //   this.setCursor();
    //   e.preventDefault();
    // }
  }

  private static onKeyDown(e: KeyboardEvent) {
    this.onKey(e);

    // if (this.down && this._tool.keypress) this._tool.keypress(e);

    this.m_listeners.keydown(e);
  }

  private static onKeyUp(e: KeyboardEvent) {
    this.onKey(e, true);

    // if (this.down && this._tool.keypress) this._tool.keypress(e);

    this.m_listeners.keyup(e);
  }

  //* Helper Events
  private static calculateDeviceType(e: PointerEvent) {
    if (this.m_type !== e.pointerType) {
      this.m_type = e.pointerType as typeof this.m_type;
      this.unmount();
      this.mount();
    }
  }

  //* Pointer Events
  private static onPointerDown(e: PointerEvent) {
    this.client.movement = vec2.create();
    this.client.position = vec2.fromValues(e.clientX, e.clientY);
    this.client.delta = vec2.create();
    this.client.origin = vec2.fromValues(e.clientX, e.clientY);

    this.scene.movement = vec2.create();
    this.scene.position = SceneManager.clientToScene(this.client.position);
    this.scene.delta = vec2.create();
    this.scene.origin = vec2.clone(this.scene.position);

    this.setPointer(e);
    this.down = true;
    this.m_abort = false;

    if (this.button === BUTTONS.MIDDLE) {
      this.activeTool = this.keys.ctrlStateChanged ? 'zoom' : 'pan';
    }

    // this.lockCursor();

    const callbacks = this.m_tool.onPointerDown();
    this.m_tool.onPointerMove = callbacks.onPointerMove;
    this.m_tool.onPointerUp = callbacks.onPointerUp;

    // const [pointermove, pointerup, keypress] = this._tool.pointerdown({
    //   input: this,
    //   elements: this._elements,
    //   state: this._state,
    //   api: this._api,
    //   setState: this.setState,
    //   render: this.render,
    //   fn: getToolData(this._tool.active)?.fn
    // });

    // this._tool.pointermove = pointermove;
    // this._tool.pointerup = pointerup;
    // this._tool.keypress = keypress;

    this.m_listeners.pointerdown(e);
  }

  private static onPointerMove(e: PointerEvent) {
    this.client.movement = vec2.sub([e.clientX, e.clientY], this.client.position);
    this.client.position = vec2.fromValues(e.clientX, e.clientY);
    this.client.delta = vec2.sub(this.client.position, this.client.origin);

    this.scene.movement = vec2.div(this.client.movement, SceneManager.viewport.zoom);
    this.scene.position = SceneManager.clientToScene(this.client.position);
    this.scene.delta = vec2.sub(this.scene.position, this.scene.origin);

    this.setPointer(e);

    if (
      !this.m_moving &&
      this.down
      //   this._tool.active !== 'pencil' &&
      //   this._tool.active !== 'pan'
    ) {
      if (
        vec2.length(this.client.delta) >
        INPUT_MOVEMENT_THRESHOLD * INPUT_MOVEMENT_THRESHOLD_MULTIPLIER[this.m_type]
      ) {
        this.m_moving = true;
      } else return;
    }

    // if (this.down && this._tool.pointermove && !this._abort) this._tool.pointermove(e);

    if (this.m_moving && this.m_tool.onPointerMove) {
      this.m_tool.onPointerMove();
      SceneManager.render();
    }

    this.m_listeners.pointermove(e);
  }

  private static onPointerUp(e: PointerEvent) {
    this.setPointer(e);
    if (!this.down) return;
    this.down = false;
    this.m_moving = false;

    // this.unlockCursor();

    if (this.button === BUTTONS.MIDDLE || this.m_shouldResetTool) {
      this.activeTool = this.m_tool.current;
    } else {
      this.calculateTool();
    }

    if (this.m_tool.onPointerUp) this.m_tool.onPointerUp(this.m_abort);

    // this.updateInputs();

    this.m_listeners.pointerup(e);
  }

  private static onPointerEnter(e: PointerEvent) {
    this.inside = true;
    this.m_listeners.pointerenter(e);
  }

  private static onPointerLeave(e: PointerEvent) {
    this.inside = false;
    this.m_listeners.pointerleave(e);
  }

  //* Touch Events
  private static onTouchStart(e: TouchEvent) {}

  private static onTouchMove(e: TouchEvent) {}

  private static onTouchEnd(e: TouchEvent) {}

  //* Window Events
  private static onResize(e: UIEvent) {
    Renderer.resize();
    SceneManager.render();
    this.m_listeners.resize(e);
  }

  private static onUnload(e: Event) {
    this.m_listeners.unload(e);
  }

  private static onWheel(e: WheelEvent) {
    e.preventDefault();
    if (e.target !== Renderer.canvas) return;
    this.m_listeners.wheel(e);
  }
}

export default InputManager;
