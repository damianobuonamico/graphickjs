import Element from '@/editor/ecs/entities/element';
import SceneManager from '@editor/scene';
import { vec2 } from '@math';
import CanvasStats from '../stats';
import Renderer from '../renderer';
import InputManager from '@/editor/input';
import SelectionManager from '@/editor/selection';
import ImageMedia from '@/editor/ecs/entities/image';

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
    cubic: this.cubic.bind(this),
    circle: this.circle.bind(this),
    crect: this.crect.bind(this),
    rect: this.nrect.bind(this),
    strokecolor: this.strokeColor.bind(this),
    fillcolor: this.fillColor.bind(this)
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
    this.m_stats.entity();
    this.m_stats.draw();
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

  private circle(operation: CenteredShapeDrawOp) {
    this.m_ctx.arc(operation.data[0][0], operation.data[0][1], operation.data[1], 0, Math.PI * 2);
  }

  private nrect(operation: ShapeDrawOp) {
    this.m_ctx.rect(
      operation.data[0][0],
      operation.data[0][1],
      operation.data[1][0],
      operation.data[1][1]
    );
  }

  private crect(operation: CenteredShapeDrawOp) {
    this.m_ctx.rect(
      operation.data[0][0] - operation.data[1],
      operation.data[0][1] - operation.data[1],
      2 * operation.data[1],
      2 * operation.data[1]
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

  private strokeColor(operation: ColorDrawOp) {
    this.m_ctx.strokeStyle = `rgba(${operation.data[0] * 255}, ${operation.data[1] * 255}, ${
      operation.data[2] * 255
    }, ${operation.data[3]})`;
  }

  private fillColor(operation: ColorDrawOp) {
    this.m_ctx.fillStyle = `rgba(${operation.data[0] * 255}, ${operation.data[1] * 255}, ${
      operation.data[2] * 255
    }, ${operation.data[3]})`;
  }

  public draw(drawable: Drawable) {
    drawable.operations.forEach((operation) => {
      (this.m_drawOpRegister as any)[operation.type](operation);
    });
  }

  public entity(entity: Entity) {
    if (entity.type === 'element') {
      this.element(entity as Element);
      return;
    }

    this.m_ctx.save();

    const drawable = entity.getDrawable(false);

    if ((entity as MovableEntity).transform) {
      const position = (entity as MovableEntity).transform.position;
      if ((entity as TransformableEntity).transform.rotation) {
        const boundingBox = (entity as MovableEntity).boundingBox;
        const mid = vec2.div(vec2.add(boundingBox[0], boundingBox[1]), 2);
        const translation = vec2.sub(mid, position);
        this.m_ctx.transform(
          1,
          0,
          0,
          1,
          position[0] + translation[0],
          position[1] + translation[1]
        );
        this.m_ctx.rotate((entity as TransformableEntity).transform.rotation);
        this.m_ctx.translate(-translation[0], -translation[1]);
      } else {
        this.m_ctx.transform(1, 0, 0, 1, position[0], position[1]);
      }
    }
    this.draw(drawable);

    this.m_ctx.restore();
  }

  public element(element: Element) {
    this.m_stats.entity();
    if (!SceneManager.isVisible(element)) return;

    this.m_ctx.save();
    this.m_ctx.strokeStyle = `rgba(0, 0, 0, 1.0)`;
    this.m_ctx.lineWidth = 1;
    const boundingBox = element.unrotatedBoundingBox;
    const position = element.transform.position;
    const mid = vec2.div(vec2.add(boundingBox[0], boundingBox[1]), 2);
    const translation = vec2.sub(mid, position);
    this.m_ctx.transform(1, 0, 0, 1, position[0] + translation[0], position[1] + translation[1]);
    this.m_ctx.rotate(element.transform.rotation);
    // if (element.transform.rotation !== 0) console.log(element.transform.rotation);
    this.m_ctx.translate(-translation[0], -translation[1]);
    // Debug
    this.m_ctx.fillStyle = 'rgb(50, 50, 50)';
    this.draw((element as Element).getDrawable(false));

    this.m_ctx.restore();

    if (Renderer.debugging && Renderer.debug.box) {
      if (
        (InputManager.hover.element && InputManager.hover.element.id === element.id) ||
        SelectionManager.has(element.id)
      ) {
        this.m_ctx.fillStyle = 'rgba(220, 20, 60, 0.2)';
        this.m_ctx.strokeStyle = 'rgb(220, 20, 60)';
      } else {
        this.m_ctx.fillStyle = 'rgba(0, 255, 127, 0.2)';
        this.m_ctx.strokeStyle = 'rgb(0, 255, 127)';
      }
      const box = (element as Element).boundingBox;
      this.begin();
      this.m_ctx.rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1]);
      this.fill();
      this.stroke();
    }
    this.m_stats.draw();
  }

  public image(image: ImageMedia) {
    if (!SceneManager.isVisible(image)) return;
    this.m_stats.draw();
    const boundingBox = image.unrotatedBoundingBox;
    const position = image.transform.position;
    const mid = vec2.div(vec2.add(boundingBox[0], boundingBox[1]), 2);
    const translation = vec2.sub(mid, position);

    // TODO: Bilinar Filtering

    this.m_ctx.save();
    this.m_ctx.transform(1, 0, 0, 1, position[0] + translation[0], position[1] + translation[1]);
    this.m_ctx.rotate(image.transform.rotation);
    this.m_ctx.translate(-translation[0], -translation[1]);

    this.m_ctx.drawImage(image.source, 0, 0);

    this.m_ctx.restore();

    if (Renderer.debugging && Renderer.debug.box) {
      if (
        (InputManager.hover.element && InputManager.hover.element.id === image.id) ||
        SelectionManager.has(image.id)
      ) {
        this.m_ctx.fillStyle = 'rgba(220, 20, 60, 0.2)';
        this.m_ctx.strokeStyle = 'rgb(220, 20, 60)';
      } else {
        this.m_ctx.fillStyle = 'rgba(0, 255, 127, 0.2)';
        this.m_ctx.strokeStyle = 'rgb(0, 255, 127)';
      }
      const box = image.boundingBox;
      this.begin();
      this.m_ctx.rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1]);
      this.fill();
      this.stroke();
    }
  }

  public beginOutline() {
    this.m_ctx.strokeStyle = 'rgba(49, 239, 284, 1.0)';
    this.m_ctx.fillStyle = 'rgba(49, 239, 284, 1.0)';
    this.m_ctx.lineWidth = 2 / SceneManager.viewport.zoom;
    this.begin();
  }

  public outline(entity: Entity, skipVertices: boolean = false) {
    this.m_ctx.save();
    if ((entity as MovableEntity).transform) {
      const position = (entity as Element).transform.position;
      if ((entity as TransformableEntity).transform.rotation) {
        const boundingBox = (entity as TransformableEntity).unrotatedBoundingBox;
        const mid = vec2.div(vec2.add(boundingBox[0], boundingBox[1]), 2);
        const translation = vec2.sub(mid, position);
        this.m_ctx.transform(
          1,
          0,
          0,
          1,
          position[0] + translation[0],
          position[1] + translation[1]
        );
        this.m_ctx.rotate((entity as TransformableEntity).transform.rotation);
        this.m_ctx.translate(-translation[0], -translation[1]);
      } else {
        this.m_ctx.transform(1, 0, 0, 1, position[0], position[1]);
      }
    }
    this.draw((entity as Element).getOutlineDrawable(false));
    if (!skipVertices && InputManager.tool.isVertex && entity.type === 'element')
      (entity as Element).forEach((vertex, selected) => vertex.render(selected));
    this.m_ctx.restore();
    this.m_stats.entity();
  }

  public endOutline() {
    this.stroke();
  }

  public debugging() {
    const width = 150;
    const lAlign = 5;
    const rAlign = 40;

    this.m_ctx.setTransform(1, 0, 0, 1, this.size[0] - width, 0);
    this.m_ctx.fillStyle = 'rgba(0.0, 0.0, 0.0, 0.5)';
    this.m_ctx.fillRect(0, 0, width, 170);

    this.m_ctx.textAlign = 'left';
    this.m_ctx.font = 'bold 12px Helvetica,Arial,sans-serif';
    this.m_ctx.fillStyle = 'white';
    this.m_ctx.textBaseline = 'top';

    this.m_ctx.fillStyle = this.m_stats.getColor(this.m_stats.fps[0]);
    this.m_ctx.fillText('FPS:', lAlign, 5);
    this.m_ctx.fillText(`${this.m_stats.fps[0]}  [${this.m_stats.fps[1]} MS]`, rAlign, 5);

    this.m_ctx.fillStyle = this.m_stats.getColor(this.m_stats.avg[0]);
    this.m_ctx.fillText('AVG:', lAlign, 30);
    this.m_ctx.fillText(`${this.m_stats.avg[0]}  [${this.m_stats.avg[1]} MS]`, rAlign, 30);

    this.m_ctx.fillStyle = this.m_stats.getColor(this.m_stats.min[0]);
    this.m_ctx.fillText('MIN:', lAlign, 55);
    this.m_ctx.fillText(`${this.m_stats.min[0]}  [${this.m_stats.min[1]} MS]`, rAlign, 55);

    this.m_ctx.fillStyle = 'mediumorchid';
    this.m_ctx.fillText('MEM:', lAlign, 80);
    this.m_ctx.fillText(`${this.m_stats.mem} / ${this.m_stats.heap} MB`, rAlign, 80);

    this.m_ctx.fillStyle = 'white';
    this.m_ctx.fillText('BOX:', lAlign, 105);
    this.m_ctx.fillText(`${Renderer.debug.box.toString().toUpperCase()}`, rAlign, 105);

    this.m_ctx.fillText('ENT:', lAlign, 130);
    this.m_ctx.fillText(`${this.m_stats.entities}`, rAlign, 130);

    this.m_ctx.fillText('DRW:', lAlign, 155);
    this.m_ctx.fillText(`${this.m_stats.drawn}`, rAlign, 155);
  }
}

export default Canvas2D;
