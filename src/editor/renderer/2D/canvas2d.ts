import SceneManager from '@/editor/scene';
import { vec2 } from '@math';

class Canvas2D implements Canvas {
  private m_container: HTMLDivElement;
  private m_canvas: HTMLCanvasElement;
  private m_ctx: CanvasRenderingContext2D;

  private m_resolution = 1;
  private m_offset: vec2;

  constructor() {}

  get container() {
    return this.m_container;
  }

  set container(div: HTMLDivElement) {
    this.m_container = div;
    this.resize();
  }

  get DOM() {
    return this.m_canvas;
  }

  get offset(): vec2 {
    return vec2.clone(this.m_offset);
  }

  set offset(value: vec2) {
    this.m_offset = value;
    this.m_canvas.style.left = value[0] + 'px';
    this.m_canvas.style.top = value[1] + 'px';
  }

  get size(): vec2 {
    return [this.m_canvas.width, this.m_canvas.height];
  }

  set size(value: vec2) {
    this.m_canvas.width = value[0] * this.m_resolution;
    this.m_canvas.height = value[1] * this.m_resolution;

    this.m_canvas.style.width = value[0] + 'px';
    this.m_canvas.style.height = value[1] + 'px';
  }

  setup(canvas: HTMLCanvasElement) {
    this.m_canvas = canvas;
    this.m_ctx = canvas.getContext('2d')!;
  }

  resize() {
    this.offset = [this.m_container.offsetLeft, this.m_container.offsetTop];
    this.size = [this.m_container.offsetWidth, this.m_container.offsetHeight];
    SceneManager.render();
  }

  public clear({ color }: { color: vec4 }) {
    this.m_ctx.clearRect(0, 0, this.size[0], this.size[1]);
    this.m_ctx.fillStyle = `rgba(${color[0] * 255}, ${color[1] * 255}, ${color[2] * 255}, ${
      color[3]
    })`;
    this.m_ctx.fillRect(0, 0, this.size[0], this.size[1]);
  }

  public beginFrame(): void {
    this.m_ctx.setTransform();
    this.clear({ color: [0.09, 0.11, 0.13, 1.0] });
    this.m_ctx.scale(SceneManager.viewport.zoom, SceneManager.viewport.zoom);
    this.m_ctx.translate(SceneManager.viewport.position[0], SceneManager.viewport.position[1]);
  }

  public endFrame(): void {}

  public rect({
    pos,
    size,
    centered = false,
    color,
    transform
  }: {
    pos: vec2;
    size: vec2 | number;
    centered?: boolean;
    color: vec4;
    transform?: mat4;
  }) {
    size = typeof size === 'number' ? [size, size] : size;
    const translate = centered ? vec2.mul(size, 0.5) : vec2.create();

    this.m_ctx.save();

    if (transform) this.m_ctx.transform(1, 0, 0, 1, transform[12], transform[13]);

    this.m_ctx.fillStyle = `rgba(${color[0] * 255}, ${color[1] * 255}, ${color[2] * 255}, ${
      color[3]
    })`;
    this.m_ctx.fillRect(pos[0] - translate[0], pos[1] - translate[1], size[0], size[1]);

    this.m_ctx.restore();
  }
}

export default Canvas2D;
