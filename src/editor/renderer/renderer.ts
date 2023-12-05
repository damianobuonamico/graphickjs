import { vec2 } from "@/math";
import API from "@/wasm/loader";

abstract class Renderer {
  private static m_canvas: HTMLCanvasElement;
  private static m_container: HTMLDivElement;
  private static m_offset: vec2 = [0, 0];
  private static m_dpr: number = 1;

  static setup(canvas: HTMLCanvasElement) {
    this.m_container = <HTMLDivElement>canvas.parentElement;
    this.m_canvas = canvas;

    setTimeout(() => {
      canvas.style.opacity = "1";
    }, 50);

    setTimeout(() => {
      this.resize();
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
  }
}

export default Renderer;
