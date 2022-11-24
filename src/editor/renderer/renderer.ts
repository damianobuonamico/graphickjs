import SceneManager from '../scene';
import Canvas2D from './2D/canvas2d';
import Stats from './stats';

abstract class Renderer {
  private static m_canvas: Canvas;

  static debugging: boolean = false;
  static debug: DebugState = {
    entityBox: true,
    segmentBox: false,
    vertices: false,
    opacity: 1
  };

  static stats: RendererStats = new Stats();

  static setup: Canvas['setup'];
  static debugRect: Canvas['debugRect'];
  static debugCircle: Canvas['debugCircle'];
  static draw: Canvas['draw'];
  static entity: Canvas['entity'];
  static element: Canvas['element'];
  static image: Canvas['image'];
  static rect: Canvas['rectangle'];
  static beginOutline: Canvas['beginOutline'];
  static outline: Canvas['outline'];
  static endOutline: Canvas['endOutline'];

  static init() {
    this.m_canvas = new Canvas2D();
    this.refresh();
  }

  static refresh() {
    this.bind(this.m_canvas);
  }

  static bind(canvas: Canvas) {
    this.setup = canvas.setup.bind(canvas);
    this.debugRect = canvas.debugRect.bind(canvas);
    this.debugCircle = canvas.debugCircle.bind(canvas);
    this.draw = canvas.draw.bind(canvas);
    this.entity = canvas.entity.bind(canvas);
    this.element = canvas.element.bind(canvas);
    this.image = canvas.image.bind(canvas);
    this.rect = canvas.rectangle.bind(canvas);
    this.beginOutline = canvas.beginOutline.bind(canvas);
    this.outline = canvas.outline.bind(canvas);
    this.endOutline = canvas.endOutline.bind(canvas);
  }

  static get canvas() {
    return this.m_canvas.DOM;
  }

  static get canvasOffset() {
    return this.m_canvas.offset;
  }

  static get size() {
    return this.m_canvas.size;
  }

  static set primaryColor(color: string) {
    this.m_canvas.primaryColor = color;
  }

  static get primaryColor() {
    return this.m_canvas.primaryColor;
  }

  static beginFrame(options: { color?: string; zoom?: number; position?: vec2 }) {
    this.m_canvas.beginFrame({ ...options, debugging: this.debugging, stats: this.stats });
  }

  static endFrame() {
    this.m_canvas.endFrame({ debugging: this.debugging, debug: this.debug, stats: this.stats });
  }

  static resize() {
    this.m_canvas.resize();
    SceneManager.setViewportArea();
  }
}

export default Renderer;
