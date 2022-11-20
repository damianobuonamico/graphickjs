import Canvas2D from './2D/canvas2d';
import Stats from './stats';

abstract class Renderer {
  private static m_canvas: Canvas;

  public static debugging: boolean = true;
  public static debug: DebugState = {
    entityBox: false,
    segmentBox: false,
    vertices: false,
    opacity: 1
  };

  public static stats: RendererStats = new Stats();

  public static setup: Canvas['setup'];
  public static resize: Canvas['resize'];
  public static debugRect: Canvas['debugRect'];
  public static debugCircle: Canvas['debugCircle'];
  public static draw: Canvas['draw'];
  public static entity: Canvas['entity'];
  public static element: Canvas['element'];
  public static image: Canvas['image'];
  public static rect: Canvas['rectangle'];
  public static beginOutline: Canvas['beginOutline'];
  public static outline: Canvas['outline'];
  public static endOutline: Canvas['endOutline'];

  public static init() {
    this.m_canvas = new Canvas2D();
    this.refresh();
  }

  public static refresh() {
    this.bind(this.m_canvas);
  }

  static bind(canvas: Canvas) {
    this.setup = canvas.setup.bind(canvas);
    this.resize = canvas.resize.bind(canvas);
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

  public static get canvas() {
    return this.m_canvas.DOM;
  }

  public static get canvasOffset() {
    return this.m_canvas.offset;
  }

  public static get size() {
    return this.m_canvas.size;
  }

  static beginFrame(options: { color?: string; zoom?: number; position?: vec2 }) {
    this.m_canvas.beginFrame({ ...options, debugging: this.debugging, stats: this.stats });
  }

  static endFrame() {
    this.m_canvas.endFrame({ debugging: this.debugging, debug: this.debug, stats: this.stats });
  }
}

export default Renderer;
