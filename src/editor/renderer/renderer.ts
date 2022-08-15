import Element from '../ecs/element';
import Canvas2D from './2D/canvas2d';
import CanvasGL from './WebGL/canvasgl';

abstract class Renderer {
  private static m_canvas: Canvas;

  public static debugging: boolean = true;
  public static debug: DebugState = {
    box: false
  };

  public static get canvas() {
    return this.m_canvas.DOM;
  }

  public static get canvasOffset() {
    return this.m_canvas.offset;
  }

  public static get size() {
    return this.m_canvas.size;
  }

  public static set container(div: HTMLDivElement) {
    this.m_canvas.container = div;
  }

  public static init(useWebGL = true) {
    this.m_canvas = useWebGL ? new CanvasGL() : new Canvas2D();
  }

  public static setup(canvas: HTMLCanvasElement) {
    this.m_canvas.setup(canvas);
  }

  public static resize() {
    this.m_canvas.resize();
  }

  public static beginFrame() {
    this.m_canvas.beginFrame();
  }

  public static endFrame() {
    this.m_canvas.endFrame();
    if (this.debugging) this.m_canvas.debugging();
  }

  public static rect({
    pos,
    size,
    color,
    centered = false,
    transform
  }: {
    pos: vec2;
    size: vec2;
    color: vec4;
    centered?: boolean;
    transform?: mat4;
  }) {
    this.m_canvas.rect({ pos, size, centered, color, transform });
  }

  public static element(element: Element) {
    this.m_canvas.element(element);
  }

  public static beginOutline() {
    this.m_canvas.beginOutline();
  }

  public static outline(entity: Entity) {
    this.m_canvas.outline(entity);
  }

  public static endOutline() {
    this.m_canvas.endOutline();
  }
}

export default Renderer;
