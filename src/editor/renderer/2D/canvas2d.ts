import Element from '@editor/ecs/element';
import SceneManager from '@editor/scene';
import { vec2 } from '@math';
import CanvasStats from '../stats';

class Canvas2D implements Canvas {
  private m_container: HTMLDivElement;
  private m_canvas: HTMLCanvasElement;
  private m_ctx: CanvasRenderingContext2D;
  private m_stats: CanvasStats;

  private m_resolution = 1;
  private m_offset: vec2;

  private m_drawOpRegister = {
    begin: this.begin.bind(this),
    close: this.close.bind(this),
    stroke: this.stroke.bind(this),
    fill: this.fill.bind(this),
    move: this.move.bind(this),
    linear: this.line.bind(this),
    quadratic: this.quadratic.bind(this),
    cubic: this.cubic.bind(this)
  };

  constructor() {
    this.m_stats = new CanvasStats();
  }

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
    this.m_stats.begin();
    this.m_ctx.setTransform();
    this.clear({ color: [0.09, 0.11, 0.13, 1.0] });
    this.m_ctx.scale(SceneManager.viewport.zoom, SceneManager.viewport.zoom);
    this.m_ctx.translate(SceneManager.viewport.position[0], SceneManager.viewport.position[1]);
    this.m_stats.clear();
  }

  public endFrame(): void {
    this.m_stats.end();
  }

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

  private begin() {
    this.m_ctx.beginPath();
  }

  private close() {
    this.m_ctx.closePath();
  }

  private stroke() {
    this.m_ctx.stroke();
  }

  private fill() {
    this.m_ctx.fill();
  }

  private move(operation: BezierDrawOp) {
    this.m_ctx.moveTo(operation.data[0][0], operation.data[0][1]);
  }

  private line(operation: BezierDrawOp) {
    this.m_ctx.lineTo(operation.data[0][0], operation.data[0][1]);
  }

  private quadratic(operation: BezierDrawOp) {
    this.m_ctx.quadraticCurveTo(
      operation.data[0][0],
      operation.data[0][1],
      operation.data[1][0],
      operation.data[1][1]
    );
  }

  private cubic(operation: BezierDrawOp) {
    this.m_ctx.bezierCurveTo(
      operation.data[0][0],
      operation.data[0][1],
      operation.data[1][0],
      operation.data[1][1],
      operation.data[2][0],
      operation.data[2][1]
    );
  }

  private draw(drawable: Drawable) {
    drawable.operations.forEach((operation) => {
      (this.m_drawOpRegister as any)[operation.type](operation);
    });
  }

  public element(element: Entity) {
    if (!element.visible) return;
    this.m_ctx.save();
    this.m_ctx.strokeStyle = `rgba(0, 0, 0, 1.0)`;
    this.m_ctx.lineWidth = 1;
    const position = (element as Element).position;
    const transform = (element as Element).transform;
    this.m_ctx.transform(1, 0, 0, 1, position[0] + transform[12], position[1] + transform[13]);
    this.draw((element as Element).getDrawable(false));
    const box = (element as Element).boundingBox;
    this.m_ctx.strokeRect(
      box[0][0] - position[0],
      box[0][1] - position[1],
      box[1][0] - box[0][0],
      box[1][1] - box[0][1]
    );
    this.m_ctx.restore();
    this.m_stats.element();
  }

  public beginOutline() {
    this.m_ctx.strokeStyle = 'rgba(49, 239, 284, 1.0)';
    this.m_ctx.lineWidth = 2 / SceneManager.viewport.zoom;
    this.begin();
  }

  public outline(entity: Entity) {
    this.m_ctx.save();
    const position = (entity as Element).position;
    const transform = (entity as Element).transform;
    this.m_ctx.transform(1, 0, 0, 1, position[0] + transform[12], position[1] + transform[13]);
    this.draw((entity as Element).getOutlineDrawable(false));
    this.m_ctx.restore();
    this.m_stats.entity();
  }

  public endOutline() {
    this.stroke();
  }

  public statistics() {
    const width = 170;
    const lAlign = 5;
    const rAlign = width - lAlign;

    this.m_ctx.setTransform(1, 0, 0, 1, this.size[0] - width, 0);
    this.m_ctx.fillStyle = 'rgba(0.0, 0.0, 0.0, 0.5)';
    this.m_ctx.fillRect(0, 0, width, 150);

    this.m_ctx.textAlign = 'left';
    this.m_ctx.font = '12px sans-serif';
    this.m_ctx.fillStyle = 'white';
    this.m_ctx.textBaseline = 'top';
    this.m_ctx.fillText('Frame Rate', lAlign, 5);
    this.m_ctx.fillText('Avg Frame Rate', lAlign, 30);
    this.m_ctx.fillText('Frame Time', lAlign, 55);
    this.m_ctx.fillText('Avg Frame Time', lAlign, 80);
    this.m_ctx.fillText('Max Frame Time', lAlign, 105);
    this.m_ctx.fillText('Min Frame Rate', lAlign, 130);
    this.m_ctx.fillStyle = 'rgba(49, 239, 284, 1.0)';

    this.m_ctx.textAlign = 'right';
    this.m_ctx.fillText(this.m_stats.fps.toString() + 'fps', rAlign, 5);
    this.m_ctx.fillText(this.m_stats.avgFps.toString() + 'fps', rAlign, 30);
    this.m_ctx.fillText(this.m_stats.frameTime.toString() + 'ms', rAlign, 55);
    this.m_ctx.fillText(this.m_stats.avgFrameTime.toString() + 'ms', rAlign, 80);
    if (this.m_stats.minFps < 30) this.m_ctx.fillStyle = 'red';
    else if (this.m_stats.minFps < 60) this.m_ctx.fillStyle = 'orange';
    this.m_ctx.fillText(this.m_stats.maxFrameTime.toString() + 'ms', rAlign, 105);
    this.m_ctx.fillText(this.m_stats.minFps.toString() + 'fps', rAlign, 130);
  }
}

export default Canvas2D;
