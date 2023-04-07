import SceneManager from "../scene";
import Stats from "./stats";
import { vec2, vec4 } from "@/math";
import API from "@/wasm/loader";

abstract class Renderer {
  private static m_canvas: HTMLCanvasElement;
  private static m_container: HTMLDivElement;
  private static m_offset: vec2 = [0, 0];
  private static m_dpr: number = 1;

  private static m_primaryColor: vec4 = [0.22, 0.76, 0.95, 1.0];

  static debugging: boolean = true;
  static debug: DebugState = {
    entityBox: false,
    segmentBox: false,
    vertices: false,
    opacity: 1,
  };

  static stats: RendererStats = new Stats();

  static debugRect: Canvas["debugRect"];
  static debugCircle: Canvas["debugCircle"];
  static debugPoints: Canvas["debugPoints"];
  static draw: Canvas["draw"];
  static entity: Canvas["entity"];
  static element: Canvas["element"];
  static image: Canvas["image"];
  static freehand: Canvas["freehand"];
  static rect: Canvas["rectangle"];
  static beginOutline: Canvas["beginOutline"];
  static outline: Canvas["outline"];
  static endOutline: Canvas["endOutline"];

  static setup(canvas: HTMLCanvasElement) {
    this.m_container = <HTMLDivElement>canvas.parentElement;
    this.m_canvas = canvas;

    setTimeout(() => {
      this.resize();
      canvas.style.opacity = "1";
    }, 100);

    setTimeout(() => {
      this.resize();
    }, 1000);
  }

  static get canvas() {
    return this.m_canvas;
  }

  static get offset(): vec2 {
    return vec2.clone(this.m_offset);
  }

  static get size(): vec2 {
    return [
      this.m_canvas.width * this.m_dpr,
      this.m_canvas.height * this.m_dpr,
    ];
  }

  static set primaryColor(color: vec4) {
    this.m_primaryColor = vec4.clone(color);
  }

  static get primaryColor() {
    return vec4.clone(this.primaryColor);
  }

  static resize() {
    this.m_dpr = window.devicePixelRatio;
    this.m_offset = [this.m_canvas.offsetLeft, this.m_canvas.offsetTop];

    const size: vec2 = [
      this.m_container.offsetWidth,
      this.m_container.offsetHeight,
    ];

    this.m_canvas.style.width = size[0] + "px";
    this.m_canvas.style.height = size[1] + "px";

    const dprSize = vec2.mulS(size, this.m_dpr);

    this.m_canvas.width = dprSize[0];
    this.m_canvas.height = dprSize[1];

    API._on_resize_event(...size, this.m_dpr, ...this.m_offset);
    SceneManager.setViewportArea();

    return size;
  }
}

export default Renderer;
