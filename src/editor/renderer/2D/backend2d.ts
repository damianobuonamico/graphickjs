import { vec2 } from '@/math';
import { MATH_TWO_PI } from '@/utils/constants';

class CanvasBackend2D {
  private m_container: HTMLDivElement;
  protected m_canvas: HTMLCanvasElement;
  protected m_ctx: CanvasRenderingContext2D;

  private m_offset: vec2;
  protected m_dpr = 1;

  protected beginPath: () => void;
  protected closePath: () => void;
  protected stroke: () => void;
  protected fill: () => void;

  private m_drawOpRegister: { [key: string]: (...args: any) => void } = {
    beginPath: () => {},
    closePath: () => {},
    stroke: () => {},
    fill: () => {},
    moveTo: (position: vec2) => {},
    lineTo: (position: vec2) => {},
    cubicTo: (position: vec2, size: vec2) => {},
    rect: (position: vec2, size: vec2) => {},
    roundedRect: (position: vec2, size: vec2, radius: number) => {},
    circle: (position: vec2, size: vec2) => {},
    square: (position: vec2, size: vec2) => {},
    strokeColor: (color: string) => {},
    fillColor: (color: string) => {},
    strokeWidth: (color: string) => {}
  };

  constructor() {}

  get DOM() {
    return this.m_canvas;
  }

  get offset(): vec2 {
    return vec2.clone(this.m_offset);
  }

  get size(): vec2 {
    return [this.m_canvas.width * this.m_dpr, this.m_canvas.height * this.m_dpr];
  }

  private bind() {
    this.beginPath = this.m_ctx.beginPath.bind(this.m_ctx);
    this.closePath = this.m_ctx.closePath.bind(this.m_ctx);
    this.stroke = this.m_ctx.stroke.bind(this.m_ctx);
    this.fill = this.m_ctx.fill.bind(this.m_ctx);

    this.m_drawOpRegister = {
      beginPath: this.beginPath,
      closePath: this.closePath,
      stroke: this.stroke,
      fill: this.fill,
      moveTo: this.moveTo.bind(this),
      lineTo: this.lineTo.bind(this),
      cubicTo: this.cubicTo.bind(this),
      rect: this.rect.bind(this),
      roundedRect: this.roundedRect.bind(this),
      circle: this.circle.bind(this),
      square: this.square.bind(this),
      strokeColor: this.strokeColor.bind(this),
      fillColor: this.fillColor.bind(this),
      strokeWidth: this.strokeWidth.bind(this)
    };
  }

  protected clear({ color }: { color?: string }) {
    const size = this.size;

    this.m_ctx.clearRect(0, 0, ...size);

    if (!color) return;

    this.m_ctx.fillStyle = color;
    this.m_ctx.fillRect(0, 0, ...size);
  }

  protected moveTo(position: vec2) {
    this.m_ctx.moveTo(...position);
  }

  protected lineTo(position: vec2) {
    this.m_ctx.lineTo(...position);
  }

  protected cubicTo(cp1: vec2, cp2: vec2, position: vec2) {
    this.m_ctx.bezierCurveTo(...cp1, ...cp2, ...position);
  }

  protected rect(position: vec2, size: vec2) {
    this.m_ctx.rect(...position, ...size);
  }

  protected roundedRect(position: vec2, size: vec2, radius: number) {
    const [x, y] = position;
    const [w, h] = size;

    if (w < 2 * radius) radius = w / 2;
    if (h < 2 * radius) radius = h / 2;

    this.m_ctx.beginPath();
    this.m_ctx.moveTo(x + radius, y);
    this.m_ctx.arcTo(x + w, y, x + w, y + h, radius);
    this.m_ctx.arcTo(x + w, y + h, x, y + h, radius);
    this.m_ctx.arcTo(x, y + h, x, y, radius);
    this.m_ctx.arcTo(x, y, x + w, y, radius);
    this.m_ctx.closePath();
  }

  protected circle(position: vec2, radius: number) {
    this.m_ctx.arc(...position, radius, 0, MATH_TWO_PI);
  }

  protected square(position: vec2, semiSize: number) {
    this.m_ctx.rect(position[0] - semiSize, position[1] - semiSize, (semiSize *= 2), semiSize);
  }

  protected fillColor(color: string) {
    this.m_ctx.fillStyle = color;
  }

  protected strokeColor(color: string) {
    this.m_ctx.strokeStyle = color;
  }

  protected strokeWidth(width: number) {
    this.m_ctx.lineWidth = width;
  }

  protected transform(matrix: mat3) {
    this.m_ctx.transform(matrix[0], matrix[3], matrix[1], matrix[4], matrix[2], matrix[5]);
  }

  setup(canvas: HTMLCanvasElement) {
    this.m_container = canvas.parentElement as HTMLDivElement;
    this.m_canvas = canvas;

    this.m_ctx = canvas.getContext('2d', {
      alpha: false,
      desinchronized: true,
      willReadFrequently: false
    })! as CanvasRenderingContext2D;

    this.bind();
  }

  resize() {
    this.m_dpr = window.devicePixelRatio;

    this.m_offset = [this.m_canvas.offsetLeft, this.m_canvas.offsetTop];

    this.m_canvas.style.width = this.m_container.offsetWidth + 'px';
    this.m_canvas.style.height = this.m_container.offsetHeight + 'px';

    this.m_canvas.width = this.m_container.offsetWidth * this.m_dpr;
    this.m_canvas.height = this.m_container.offsetHeight * this.m_dpr;
  }

  beginFrame({
    color,
    zoom = 1,
    position = [0, 0],
    stats
  }: {
    color?: string;
    zoom?: number;
    position?: vec2;
    stats?: RendererStats;
  }): void {
    stats?.begin();

    this.m_ctx.resetTransform();

    this.clear({ color });

    this.m_ctx.scale((zoom *= this.m_dpr), zoom);
    this.m_ctx.translate(...position);
  }

  endFrame({ stats }: { stats?: RendererStats }) {
    stats?.end();
  }

  debugRect({
    position,
    size = [10, 10],
    centered = true,
    color = '#DC143C'
  }: {
    position: vec2;
    size?: vec2 | number;
    centered?: boolean;
    color?: string;
  }) {
    size = typeof size === 'number' ? [size, size] : size;
    position = centered ? vec2.sub(position, vec2.divS(size, 2)) : position;

    this.m_ctx.fillStyle = color;
    this.m_ctx.fillRect(...position, ...size);
  }

  debugCircle({
    position,
    radius = 5,
    color = '#DC143C'
  }: {
    position: vec2;
    radius?: number;
    color?: string;
  }) {
    this.m_ctx.fillStyle = color;
    this.beginPath();
    this.m_ctx.arc(...position, radius, 0, MATH_TWO_PI);
    this.fill();
  }

  draw(drawable: Drawable) {
    for (let i = 0, n = drawable.operations.length; i < n; ++i) {
      this.m_drawOpRegister[drawable.operations[i].type](...(drawable.operations[i].data || []));
    }
  }

  rectangle({
    position,
    size,
    centered,
    stroke,
    fill
  }: {
    position: vec2;
    size: vec2 | number;
    centered?: boolean;
    stroke?: string;
    fill?: string;
  }) {
    size = typeof size === 'number' ? [size, size] : size;
    position = centered ? vec2.sub(position, vec2.divS(size, 2)) : position;

    this.beginPath();
    this.m_ctx.rect(...position, ...size);

    if (fill) {
      this.m_ctx.fillStyle = fill;
      this.fill();
    }

    if (stroke) {
      this.m_ctx.strokeStyle = stroke;
      this.stroke();
    }
  }
}

export default CanvasBackend2D;
