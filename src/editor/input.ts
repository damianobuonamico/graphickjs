import {
  INPUT_MOVEMENT_THRESHOLD,
  INPUT_MOVEMENT_THRESHOLD_MULTIPLIER,
  ZOOM_STEP
} from '@utils/constants';
import { BUTTONS, KEYS } from '@utils/keys';
import { fillObject, isInputLike, isShortcut } from '@utils/utils';
import { map, vec2 } from '@math';
import { Renderer } from './renderer';
import SceneManager from './scene';
import actions from './actions';
import HoverState from './hover';
import { ToolState } from './tools';
import AnimationManager from './animation/animation';
import CommandHistory from './history/history';

abstract class InputManager {
  public static target: EventTarget | undefined;

  public static client: PointerCoord;
  public static scene: PointerCoord;

  public static down: boolean = false;
  public static inside: boolean = true;

  public static button: number = 0;
  public static keys: KeysState;

  public static hover: HoverState = new HoverState();

  private static m_moving: boolean = false;
  private static m_abort: boolean = false;
  private static m_shouldResetTool: boolean = false;

  public static tool: ToolState;

  private static m_type: 'touch' | 'pen' | 'mouse' = 'mouse';

  private static m_listeners: Listeners;
  private static m_mountedListeners: MountedListener[] = [];

  public static init(listeners: Partial<Listeners>, setTool: (tool: Tool) => void) {
    this.m_listeners = fillObject<Listeners>(listeners, {
      copy: () => {},
      paste: () => {},
      cut: () => {},
      keydown: () => {},
      keyup: () => {},
      resize: () => {},
      pointerdown: () => {},
      pointermove: () => {},
      pointerup: () => {},
      pointercancel: () => {},
      pointerleave: () => {},
      pointerenter: () => {},
      wheel: () => {}
    });

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

    this.tool = new ToolState(setTool, 'select');

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
      this.addListener('touchstart', this.onTouchStart.bind(this));
      this.addListener('touchmove', this.onTouchMove.bind(this));
      this.addListener('touchend', this.onTouchEnd.bind(this));
      this.addListener('touchcancel', this.onTouchEnd.bind(this));
    } else {
      this.addListener('pointerdown', this.onPointerDown.bind(this));
      this.addListener('pointermove', this.onPointerMove.bind(this));
      this.addListener('pointerup', this.onPointerUp.bind(this));
      this.addListener('pointercancel', this.onPointerUp.bind(this));
      this.addListener('pointerenter', this.onPointerEnter.bind(this), document);
      this.addListener('pointerleave', this.onPointerLeave.bind(this), document);
    }

    this.addListener('resize', this.onResize.bind(this));
    this.addListener('contextmenu', (e: UIEvent) => e.preventDefault());
    this.addListener('wheel', this.onWheel.bind(this), window, { passive: false });
  }

  private static unmount() {
    this.m_mountedListeners.forEach((listener) => {
      listener.target.removeEventListener(listener.type, listener.callback);
    });
    this.m_mountedListeners.length = 0;
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
      this.tool.abort();
      this.m_abort = true;
      // this.unlockCursor();
      SceneManager.render();
    } else if (e.key === KEYS.SPACEBAR) {
      this.keys.spaceStateChanged = this.keys.space === up;
      this.keys.space = !up;
    }

    if (this.down) {
      this.m_shouldResetTool = false;
      return;
    }

    if (this.keys.ctrlStateChanged || this.keys.spaceStateChanged) this.tool.recalculate();

    // if (this.__alt) {
    //   this.setCursor();
    //   e.preventDefault();
    // }
  }

  private static onKeyDown(e: KeyboardEvent) {
    if (e.repeat && e.key.toUpperCase() !== KEYS.Z.toUpperCase()) return;

    this.onKey(e);

    if (this.down) this.tool.onKey(e);

    if (!this.down) {
      Object.values(actions).forEach((action) => {
        if (action.shortcut && isShortcut(e, action.shortcut)) {
          e.preventDefault();
          action.callback();
          SceneManager.render();
          AnimationManager.renderSequencer();
          return;
        }
      });
    }

    this.m_listeners.keydown(e);
  }

  private static onKeyUp(e: KeyboardEvent) {
    this.onKey(e, true);

    if (this.down) this.tool.onKey(e);

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
    CommandHistory.endBatch();

    if (e.target === AnimationManager.canvas) {
      this.target = e.target;

      this.client.movement = vec2.create();
      this.client.position = vec2.fromValues(e.clientX, e.clientY);
      this.client.delta = vec2.create();
      this.client.origin = vec2.fromValues(e.clientX, e.clientY);

      this.setPointer(e);
      this.down = true;
      this.m_abort = false;
      this.button = e.button;

      AnimationManager.onPointerDown();

      return;
    } else if (e.target !== Renderer.canvas) return;

    this.target = e.target;

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
    this.button = e.button;

    if (this.button === BUTTONS.MIDDLE) {
      this.tool.active = this.keys.ctrlStateChanged ? 'zoom' : 'pan';
    }

    // this.lockCursor();

    const hover = this.hover.entity;

    this.tool.onPointerDown(hover && hover.type === 'generichandle' && (hover as any).handleType);
    this.m_listeners.pointerdown(e);

    SceneManager.render();
  }

  private static onPointerMove(e: PointerEvent) {
    if (
      this.target === AnimationManager.canvas ||
      (!this.down && e.target === AnimationManager.canvas)
    ) {
      this.client.movement = vec2.sub([e.clientX, e.clientY], this.client.position);
      this.client.position = vec2.fromValues(e.clientX, e.clientY);
      this.client.delta = vec2.sub(this.client.position, this.client.origin);

      this.setPointer(e);

      AnimationManager.onPointerMove();

      return;
    } else if (this.target !== Renderer.canvas && e.target !== Renderer.canvas) return;

    this.client.movement = vec2.sub([e.clientX, e.clientY], this.client.position);
    this.client.position = vec2.fromValues(e.clientX, e.clientY);
    this.client.delta = vec2.sub(this.client.position, this.client.origin);

    this.scene.movement = vec2.divS(this.client.movement, SceneManager.viewport.zoom);
    this.scene.position = SceneManager.clientToScene(this.client.position);
    this.scene.delta = vec2.sub(this.scene.position, this.scene.origin);

    this.setPointer(e);

    this.hover.entity = SceneManager.getEntityAt(this.scene.position, this.tool.isVertex);

    if (!this.m_moving && this.down) {
      if (
        vec2.length(this.client.delta) >
        INPUT_MOVEMENT_THRESHOLD * INPUT_MOVEMENT_THRESHOLD_MULTIPLIER[this.m_type]
      ) {
        this.m_moving = true;
      } else return;
    }

    if (this.m_moving && !this.m_abort) {
      this.tool.onPointerMove();
      SceneManager.render();
    } else if (!this.down) {
      this.tool.onPointerHover();
    }

    this.m_listeners.pointermove(e);
  }

  private static onPointerUp(e: PointerEvent) {
    this.setPointer(e);
    if (!this.down) return;

    if (this.target === AnimationManager.canvas) {
      this.target = undefined;
      this.down = false;
      this.m_moving = false;

      AnimationManager.onPointerUp();

      return;
    }

    this.target = undefined;
    this.down = false;
    this.m_moving = false;

    // this.unlockCursor();

    if (this.button === BUTTONS.MIDDLE || this.m_shouldResetTool) {
      this.tool.active = this.tool.current;
    } else this.tool.recalculate();

    this.tool.onPointerUp(this.m_abort);

    this.m_listeners.pointerup(e);

    CommandHistory.endBatch();

    SceneManager.render();
    SceneManager.save();
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
  public static onResize(e: UIEvent) {
    Renderer.resize();
    AnimationManager.resize();
    SceneManager.render();
    this.m_listeners.resize(e);
  }

  private static onWheel(e: WheelEvent) {
    e.preventDefault();

    if (e.target === AnimationManager.canvas) {
      AnimationManager.onWheel(e);
      return;
    } else if (e.target !== Renderer.canvas) return;

    this.m_listeners.wheel(e);

    if (!this.keys.ctrl) return;

    SceneManager.zoom = [
      map(-e.deltaY, -100, 100, 1 - ZOOM_STEP / 10, 1 + ZOOM_STEP / 10) *
        SceneManager.viewport.zoom,
      InputManager.client.position
    ];

    SceneManager.render();
  }
}

export default InputManager;
