import Canvas2D from './2D/canvas2d';
import CanvasGL from './WebGL/canvasgl';

abstract class Renderer {
  private static m_canvas: Canvas;

  public static set container(div: HTMLDivElement) {
    this.m_canvas.container = div;
  }

  public static init(useWebGL = true) {
    this.m_canvas = useWebGL ? new CanvasGL() : new Canvas2D();
  }

  public static setup(canvas: HTMLCanvasElement) {
    this.m_canvas.setup(canvas);
  }

  public static beginFrame() {
    this.m_canvas.beginFrame();
    this.m_canvas.clear({ color: [0.0, 0.0, 0.1, 1.0] });
  }

  public static endFrame() {
    this.m_canvas.endFrame();
  }

  public static rect({ pos, size }: { pos: vec2; size: vec2 }) {
    this.m_canvas.rect({ pos, size, centered: false });
  }
}

export default Renderer;
