import { KEYS } from "@utils/keys";
import { fillObject, isInputLike } from "@utils/utils";
import { vec2, round } from "@math";
import { Renderer } from "./renderer";
import { ToolState } from "./tools";

import API from "@/wasm/loader";

abstract class InputManager {
  public static tool: ToolState;

  private static m_type: "touch" | "pen" | "mouse" = "mouse";
  private static m_touches: Map<number, TouchID> = new Map();
  private static m_touchCenter: vec2 = vec2.create();

  private static m_mountedListeners: MountedListener[] = [];

  public static init(setTool: (tool: Tool) => void) {
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
      API._shutdown();
    });
  }

  private static unmount() {
    this.m_mountedListeners.forEach((listener) => {
      listener.target.removeEventListener(listener.type, listener.callback);
    });
    this.m_mountedListeners.length = 0;
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
  }

  private static onCopy(e: ClipboardEvent) {
    API._on_clipboard_event(0);
  }

  private static onPaste(e: ClipboardEvent) {
    API._on_clipboard_event(1);
  }

  //* Keyboard Events
  private static onKeyDown(e: KeyboardEvent) {
    if (isInputLike(e.target)) return;

    API._on_keyboard_event(
      0,
      this.getKeyCode(e.key),
      e.repeat,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );
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
    if (isInputLike(e.target)) return;

    API._on_keyboard_event(
      1,
      this.getKeyCode(e.key),
      e.repeat,
      e.altKey,
      e.ctrlKey,
      e.shiftKey
    );
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
    Renderer.resize();
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
  }
}

export default InputManager;
