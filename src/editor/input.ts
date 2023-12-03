import {
  INPUT_MOVEMENT_THRESHOLD,
  INPUT_MOVEMENT_THRESHOLD_MULTIPLIER,
  ZOOM_STEP,
} from "@utils/constants";
import { BUTTONS, KEYS } from "@utils/keys";
import { fillObject, isInputLike, isShortcut } from "@utils/utils";
import { map, vec2, round } from "@math";
import { Renderer } from "./renderer";
import SceneManager from "./scene";
import actions from "./actions";
import HoverState from "./hover";
import { ToolState } from "./tools";
import AnimationManager from "./animation/animation";
import CommandHistory from "./history/history";
import API from "@/wasm/loader";

abstract class InputManager {
  public static target: EventTarget | undefined;

  public static client: PointerCoord;
  public static scene: PointerCoord;

  public static down: boolean = false;
  public static inside: boolean = true;
  public static pressure: number | undefined = undefined;
  public static time: number = 0;

  public static button: number = 0;
  public static keys: KeysState;

  public static hover: HoverState = new HoverState();

  private static m_moving: boolean = false;
  private static m_abort: boolean = false;
  private static m_shouldResetTool: boolean = false;

  public static tool: ToolState;

  private static m_type: "touch" | "pen" | "mouse" = "mouse";
  private static m_touches: Map<number, TouchID> = new Map();
  private static m_touchCenter: vec2 = vec2.create();

  private static m_listeners: Listeners;
  private static m_mountedListeners: MountedListener[] = [];

  public static init(
    listeners: Partial<Listeners>,
    setTool: (tool: Tool) => void
  ) {
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
      wheel: () => {},
    });

    this.client = fillObject(
      {},
      {
        position: vec2.create(),
        movement: vec2.create(),
        delta: vec2.create(),
        origin: vec2.create(),
      }
    );

    this.scene = fillObject(
      {},
      {
        position: vec2.create(),
        movement: vec2.create(),
        delta: vec2.create(),
        origin: vec2.create(),
      }
    );

    this.keys = fillObject(
      {},
      {
        ctrl: false,
        shift: false,
        alt: false,
        space: false,
      }
    );

    this.tool = new ToolState(setTool, "select");

    this.mount();
  }

  private static addListener<
    K extends keyof HTMLElementEventMap,
    T extends keyof WindowEventMap
  >(
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

    this.addListener("cut", this.onCut.bind(this));
    this.addListener("copy", this.onCopy.bind(this));
    this.addListener("paste", this.onPaste.bind(this));

    this.addListener("keydown", this.onKeyDown.bind(this));
    this.addListener("keyup", this.onKeyUp.bind(this));

    this.addListener("pointerdown", this.calculateDeviceType.bind(this));

    if (this.m_type === "touch") {
      this.addListener("touchstart", this.onTouchStart.bind(this));
      this.addListener("touchmove", this.onTouchMove.bind(this));
      this.addListener("touchend", this.onTouchEnd.bind(this));
      this.addListener("touchcancel", this.onTouchEnd.bind(this));
    } else {
      this.addListener("pointerdown", this.onPointerDown.bind(this));
      this.addListener("pointermove", this.onPointerMove.bind(this), window, {
        passive: true,
        capture: true,
      });
      this.addListener("pointerup", this.onPointerUp.bind(this));
      this.addListener("pointercancel", this.onPointerUp.bind(this));
      this.addListener(
        "pointerenter",
        this.onPointerEnter.bind(this),
        document
      );
      this.addListener(
        "pointerleave",
        this.onPointerLeave.bind(this),
        document
      );
    }

    this.addListener("resize", this.onResize.bind(this));
    // this.addListener("contextmenu", (e: UIEvent) => e.preventDefault());
    this.addListener("wheel", this.onWheel.bind(this), window, {
      passive: false,
    });
    this.addListener("load", () => {
      setTimeout(() => {
        Renderer.resize();
      }, 100);
    });
    this.addListener("beforeunload", () => {
      SceneManager.save();
      API._shutdown();
    });
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
    this.pressure = e.pressure;
    this.time = e.timeStamp;

    if (this.down) return;
  }

  private static touchById(touches: TouchList, id: number) {
    for (let i = 0; i < touches.length; i++) {
      if (touches.item(i)?.identifier === id) {
        return touches.item(i);
      }
    }

    return null;
  }

  private static updateTouches(e: TouchEvent) {
    const prevNum = this.m_touches.size;

    // Add or update active touches
    for (let i = 0; i < e.touches.length; i++) {
      if (e.touches[i].target !== Renderer.canvas) continue;

      const id = e.touches[i].identifier;
      const center = vec2.fromValues(
        e.touches[i].clientX,
        e.touches[i].clientY
      );

      if (this.m_touches.has(id)) {
        // Update existing touch
        const touch = this.m_touches.get(id)!;

        vec2.copy(touch.prev, touch.to);
        vec2.copy(touch.to, center);
      } else {
        // Add new touch
        this.m_touches.set(id, {
          position: vec2.clone(center),
          prev: vec2.clone(center),
          to: vec2.clone(center),
          id,
        });
      }
    }

    const num = this.m_touches.size;

    // Remove dropped touches
    if (num > 0) {
      for (let [id, _] of this.m_touches) {
        if (!this.touchById(e.touches, id)) {
          this.m_touches.delete(id);
        }
      }
    }

    // Return true if active touch count did not change at any stage.
    const change = prevNum != num || num != this.m_touches.size;

    if (change) {
      let accum: vec2 = vec2.create();

      for (let [_, touch] of this.m_touches) {
        vec2.add(accum, touch.position, accum);
      }

      this.m_touchCenter = vec2.divS(accum, this.m_touches.size);
    }

    return change;
  }

  private static centerDragDistance() {
    if (this.m_touches.size < 2) return vec2.create();

    let a = vec2.create();
    let b = vec2.create();

    for (let [_, touch] of this.m_touches) {
      vec2.add(a, touch.prev, a);
      vec2.add(b, touch.to, b);
    }

    vec2.divS(a, this.m_touches.size, a);
    vec2.divS(b, this.m_touches.size, b);

    return vec2.sub(a, b);
  }

  //* Clipboard Events
  private static onCut(e: ClipboardEvent) {
    API._on_clipboard_event(2);
    this.m_listeners.cut(e);
  }

  private static onCopy(e: ClipboardEvent) {
    API._on_clipboard_event(0);
    this.m_listeners.copy(e);
  }

  private static onPaste(e: ClipboardEvent) {
    API._on_clipboard_event(1);
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

    if (this.keys.ctrlStateChanged || this.keys.spaceStateChanged)
      this.tool.recalculate();

    // if (this.__alt) {
    //   this.setCursor();
    //   e.preventDefault();
    // }
  }

  private static onKeyDown(e: KeyboardEvent) {
    API._on_keyboard_event(
      0,
      this.getKeyCode(e.key),
      e.repeat,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    if (e.repeat && e.key.toUpperCase() !== KEYS.Z.toUpperCase()) return;

    this.onKey(e);

    if (this.down) this.tool.onKey(e);

    if (!this.down) {
      Object.values(actions).forEach((action) => {
        if (action.shortcut && isShortcut(e, action.shortcut)) {
          e.preventDefault();
          action.callback();
          // SceneManager.render();
          AnimationManager.renderSequencer();
          return;
        }
      });
    }

    this.m_listeners.keydown(e);
  }

  private static getKeyCode(key: string): number {
    switch (key) {
      case KEYS.BACKSPACE:
        return 8;
      case KEYS.SHIFT:
        return 16;
      case KEYS.CTRL_KEY:
        return 17;
      case KEYS.ALT:
        return 18;
      case KEYS.ESCAPE:
        return 27;
      case KEYS.SPACEBAR:
        return 32;
      case KEYS.DELETE:
        return 46;
      default: {
        if (key.length > 1) return -1;

        const code = key.toLowerCase().charCodeAt(0);

        if (code < 97 || code > 172) return -1;

        return code;
      }
    }
  }

  private static onKeyUp(e: KeyboardEvent) {
    API._on_keyboard_event(
      1,
      this.getKeyCode(e.key),
      e.repeat,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    this.onKey(e, true);

    if (this.down) this.tool.onKey(e);

    this.m_listeners.keyup(e);
  }

  //* Helper Events
  private static calculateDeviceType(e: PointerEvent) {
    if (this.m_type !== e.pointerType) {
      this.m_type = <typeof this.m_type>e.pointerType;
      this.unmount();
      this.mount();

      if (this.m_type === "touch")
        this.onTouchStart(new TouchEvent("touchdown", e));
      else this.onPointerDown(e);
    }
  }

  private static getPointerTypeCode(type: string): number {
    switch (type) {
      case "pen":
        return 1;
      case "touch":
        return 2;
      default:
      case "mouse":
        return 0;
    }
  }

  //* Pointer Events
  private static onPointerDown(e: PointerEvent) {
    API._on_pointer_event(
      e.target === Renderer.canvas ? 1 : 0,
      0,
      this.getPointerTypeCode(e.pointerType),
      e.button,
      e.clientX,
      e.clientY,
      e.pressure,
      e.timeStamp,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    CommandHistory.endBatch();

    if (
      SceneManager.state.workspace === "designer" &&
      e.target === AnimationManager.canvas
    ) {
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
      this.tool.active = this.keys.ctrlStateChanged ? "zoom" : "pan";
    } else if (
      this.button === BUTTONS.ERASER &&
      this.tool.active === "pencil" &&
      SceneManager.state.workspace === "whiteboard"
    ) {
      this.tool.active = "eraser";
    }
    // this.lockCursor();

    const hover = this.hover.entity;

    // this.tool.onPointerDown(
    //   hover && hover.type === "generichandle" && (hover as any).handleType
    // );
    this.m_listeners.pointerdown(e);

    SceneManager.render();
  }

  private static onPointerMove(e: PointerEvent) {
    API._on_pointer_event(
      e.target === Renderer.canvas ? 1 : 0,
      1,
      this.getPointerTypeCode(e.pointerType),
      e.button,
      e.clientX,
      e.clientY,
      e.pressure,
      e.timeStamp,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    if (
      (SceneManager.state.workspace === "designer" &&
        this.target === AnimationManager.canvas) ||
      (!this.down && e.target === AnimationManager.canvas)
    ) {
      this.client.movement = vec2.sub(
        [e.clientX, e.clientY],
        this.client.position
      );
      this.client.position = vec2.fromValues(e.clientX, e.clientY);
      this.client.delta = vec2.sub(this.client.position, this.client.origin);

      this.setPointer(e);

      AnimationManager.onPointerMove();

      return;
    } else if (this.target !== Renderer.canvas && e.target !== Renderer.canvas)
      return;

    this.client.movement = vec2.sub(
      [e.clientX, e.clientY],
      this.client.position
    );
    this.client.position = vec2.fromValues(e.clientX, e.clientY);
    this.client.delta = vec2.sub(this.client.position, this.client.origin);

    this.scene.movement = vec2.divS(
      this.client.movement,
      SceneManager.viewport.zoom
    );
    this.scene.position = SceneManager.clientToScene(this.client.position);
    this.scene.delta = vec2.sub(this.scene.position, this.scene.origin);

    this.setPointer(e);

    if (
      !(
        (this.down && this.tool.active === "pan") ||
        this.tool.active === "zoom"
      )
    )
      this.hover.entity = SceneManager.getEntityAt(
        this.scene.position,
        this.tool.isVertex
      );

    if (!this.m_moving && this.down) {
      if (
        this.tool.active === "pencil" ||
        vec2.length(this.client.delta) >
          INPUT_MOVEMENT_THRESHOLD *
            INPUT_MOVEMENT_THRESHOLD_MULTIPLIER[this.m_type]
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
    API._on_pointer_event(
      e.target === Renderer.canvas ? 1 : 0,
      2,
      this.getPointerTypeCode(e.pointerType),
      e.button,
      e.clientX,
      e.clientY,
      e.pressure,
      e.timeStamp,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    this.setPointer(e);
    if (!this.down) return;

    if (
      SceneManager.state.workspace === "designer" &&
      this.target === AnimationManager.canvas
    ) {
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
    API._on_pointer_event(
      e.target === Renderer.canvas ? 1 : 0,
      3,
      this.getPointerTypeCode(e.pointerType),
      e.button,
      e.clientX,
      e.clientY,
      e.pressure,
      e.timeStamp,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    this.inside = true;
    this.m_listeners.pointerenter(e);
  }

  private static onPointerLeave(e: PointerEvent) {
    API._on_pointer_event(
      e.target === Renderer.canvas ? 1 : 0,
      4,
      this.getPointerTypeCode(e.pointerType),
      e.button,
      e.clientX,
      e.clientY,
      e.pressure,
      e.timeStamp,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    this.inside = false;
    this.m_listeners.pointerleave(e);
  }

  //* Touch Events
  private static onTouchStart(e: TouchEvent) {
    this.updateTouches(e);

    if (e.touches.length == 1) {
      setTimeout(() => {
        if (this.m_touches.size < 2) {
          API._on_pointer_event(
            e.target === Renderer.canvas ? 1 : 0,
            0,
            this.getPointerTypeCode("touch"),
            0,
            e.touches[0].clientX,
            e.touches[0].clientY,
            e.touches[0].force,
            e.timeStamp,
            e.altKey,
            e.ctrlKey,
            e.shiftKey
          );
        }
      }, 25);
    }

    if (e.target == Renderer.canvas) e.preventDefault();
  }

  private static onTouchMove(e: TouchEvent) {
    const change = this.updateTouches(e);

    if (e.touches.length == 1) {
      API._on_pointer_event(
        e.target === Renderer.canvas ? 1 : 0,
        1,
        this.getPointerTypeCode("touch"),
        0,
        e.touches[0].clientX,
        e.touches[0].clientY,
        e.touches[0].force,
        e.timeStamp,
        e.altKey,
        e.ctrlKey,
        e.shiftKey
      );
    } else if (this.m_touches.size == 2 && !change) {
      const it: any = this.m_touches.values();
      const touch1 = it.next().value;
      const touch2 = it.next().value;

      const prevDistance = vec2.dist(touch1.prev, touch2.prev);
      const distance = vec2.dist(touch1.to, touch2.to);

      const distanceDelta = Math.abs(prevDistance - distance);

      if (distanceDelta > Number.EPSILON && prevDistance > Number.EPSILON) {
        const scaleDelta = distance / prevDistance;

        API._on_touch_pinch(
          e.target === Renderer.canvas ? 1 : 0,
          scaleDelta,
          this.m_touchCenter[0],
          this.m_touchCenter[1]
        );
      }

      const min = vec2.neg(this.centerDragDistance());

      API._on_touch_drag(e.target === Renderer.canvas ? 1 : 0, min[0], min[1]);

      vec2.add(this.m_touchCenter, min, this.m_touchCenter);
    }

    if (e.target == Renderer.canvas) e.preventDefault();
  }

  private static onTouchEnd(e: TouchEvent) {
    API._on_pointer_event(
      e.target === Renderer.canvas ? 1 : 0,
      2,
      this.getPointerTypeCode("touch"),
      0,
      this.m_touchCenter[0],
      this.m_touchCenter[1],
      0,
      e.timeStamp,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );

    this.updateTouches(e);

    if (e.target == Renderer.canvas) e.preventDefault();
  }

  //* Window Events
  public static onResize(e: UIEvent) {
    const size = Renderer.resize();
    // AnimationManager.resize();
    // SceneManager.render();
    this.m_listeners.resize(e);
  }

  private static onWheel(e: WheelEvent) {
    let deltaX = e.deltaX;
    let deltaY = e.deltaY;

    if (navigator.userAgent.match(/chrome|chromium|crios/i)) {
      deltaX *= 0.006;
      deltaY *= 0.006;
    } else {
      deltaX *= 0.009;
      deltaY *= 0.009;
    }

    if (Math.abs(round(deltaY, 3) - deltaY) > Number.EPSILON) deltaY *= 5;

    if (deltaX === 0 && e.shiftKey) {
      deltaX = deltaY;
      deltaY = 0;
    }

    API._on_wheel_event(
      e.target === Renderer.canvas ? 1 : 0,
      deltaX,
      deltaY,
      e.ctrlKey
    );

    e.preventDefault();

    if (
      SceneManager.state.workspace === "designer" &&
      e.target === AnimationManager.canvas
    ) {
      AnimationManager.onWheel(e);
      return;
    } else if (e.target !== Renderer.canvas) return;

    this.m_listeners.wheel(e);

    if (!this.keys.ctrl) return;

    SceneManager.viewport.zoom = [
      map(-e.deltaY, -100, 100, 1 - ZOOM_STEP / 10, 1 + ZOOM_STEP / 10) *
        SceneManager.viewport.zoom,
      InputManager.client.position,
    ];

    SceneManager.render();
  }
}

export default InputManager;
