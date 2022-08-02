import { INPUT_MOVEMENT_THRESHOLD, INPUT_MOVEMENT_THRESHOLD_MULTIPLIER } from '@/utils/constants';
import { KEYS } from '@/utils/keys';
import { vec2 } from '@math';
import { fillObject, isInputLike } from '@utils/utils';
import { Renderer } from './renderer';
import SceneManager from './scene';

abstract class InputManager {
  private static m_client: PointerCoord;
  private static m_scene: PointerCoord;

  private static m_down: boolean = false;
  private static m_inside: boolean = true;
  private static m_type: 'touch' | 'pen' | 'mouse' = 'mouse';

  private static m_moving: boolean = false;
  private static m_abort: boolean = false;
  private static m_shouldResetTool: boolean = false;

  private static m_button: number = 0;
  private static m_keys: KeysState;

  private static m_tool: ToolState;
  private static m_hover: Entity | undefined;

  private static m_listeners: Listeners;
  private static m_mountedListeners: MountedListener[] = [];

  public static init(listeners: Partial<Listeners>) {
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

    this.m_client = fillObject(
      {},
      {
        position: vec2.fromValues(0, 0),
        movement: vec2.fromValues(0, 0),
        delta: vec2.fromValues(0, 0),
        origin: vec2.fromValues(0, 0)
      }
    );

    this.m_scene = fillObject(
      {},
      {
        position: vec2.fromValues(0, 0),
        movement: vec2.fromValues(0, 0),
        delta: vec2.fromValues(0, 0),
        origin: vec2.fromValues(0, 0)
      }
    );

    this.m_keys = fillObject({}, {});

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

  private static setKey(e: PointerEvent | KeyboardEvent) {
    this.m_keys.altStateChanged = this.m_keys.alt !== e.altKey;
    this.m_keys.alt = e.altKey;

    this.m_keys.ctrlStateChanged = this.m_keys.ctrl !== (e as any)[KEYS.CTRL];
    this.m_keys.ctrl = (e as any)[KEYS.CTRL];

    this.m_keys.shiftStateChanged = this.m_keys.shift !== e.shiftKey;
    this.m_keys.shift = e.shiftKey;
  }

  private static setPointer(e: PointerEvent) {
    this.m_button = e.button;
    this.setKey(e);

    if (this.m_down) return;
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
      // this._abort = true;
      // this.unlockCursor();
      // this._api.ops.render();
    } else if (e.key === KEYS.SPACEBAR) {
      this.m_keys.spaceStateChanged = this.m_keys.space === up;
      this.m_keys.space = !up;
    }

    if (this.m_down) {
      this.m_shouldResetTool = true;
      return;
    }

    // if (this.__meta || this.__space) {
    //   this._calculateTool();
    // }

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
    this.m_client.movement = vec2.fromValues(0, 0);
    this.m_client.position = vec2.fromValues(e.clientX, e.clientY);
    this.m_client.delta = vec2.fromValues(0, 0);
    this.m_client.origin = vec2.fromValues(e.clientX, e.clientY);

    this.m_scene.movement = vec2.fromValues(0, 0);
    this.m_scene.position = SceneManager.clientToScene(this.m_client.position);
    this.m_scene.delta = vec2.fromValues(0, 0);
    this.m_scene.origin = vec2.clone(this.m_scene.position);

    this.setPointer(e);
    this.m_down = true;
    this.m_abort = false;

    // if (this.button === BUTTONS.MIDDLE) {
    //   this.activeTool = this.__meta ? 'zoom' : 'pan';
    // }

    // this.lockCursor();

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
    this.m_client.movement = vec2.sub([e.clientX, e.clientY], this.m_client.position);
    this.m_client.position = vec2.fromValues(e.clientX, e.clientY);
    this.m_client.delta = vec2.sub(this.m_client.position, this.m_client.origin);

    this.m_scene.movement = vec2.div(this.m_client.movement, SceneManager.viewport.zoom);
    this.m_scene.position = SceneManager.clientToScene(this.m_client.position);
    this.m_scene.delta = vec2.sub(this.m_scene.position, this.m_scene.origin);

    this.setPointer(e);

    if (
      !this.m_moving &&
      this.m_down
      //   this._tool.active !== 'pencil' &&
      //   this._tool.active !== 'pan'
    ) {
      if (
        vec2.length(this.m_client.delta) >
        INPUT_MOVEMENT_THRESHOLD * INPUT_MOVEMENT_THRESHOLD_MULTIPLIER[this.m_type]
      ) {
        this.m_moving = true;
      } else return;
    }

    // if (this.down && this._tool.pointermove && !this._abort) this._tool.pointermove(e);

    if (this.m_moving) {
      SceneManager.viewport.position = vec2.add(
        SceneManager.viewport.position,
        this.m_client.movement
      );
      SceneManager.render();
    }

    this.m_listeners.pointermove(e);
  }

  private static onPointerUp(e: PointerEvent) {
    this.setPointer(e);
    if (!this.m_down) return;
    this.m_down = false;
    this.m_moving = false;

    // this.unlockCursor();

    // if (this.button === BUTTONS.MIDDLE || this._sholdResetTool) {
    //   this.resetTool();
    // }

    // if (this._tool.pointerup) this._tool.pointerup(e, this._abort);

    // this.updateInputs();

    this.m_listeners.pointerup(e);
  }

  private static onPointerEnter(e: PointerEvent) {
    this.m_inside = true;
    this.m_listeners.pointerenter(e);
  }

  private static onPointerLeave(e: PointerEvent) {
    this.m_inside = false;
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
