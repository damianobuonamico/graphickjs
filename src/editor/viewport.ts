import { clamp, round, vec2 } from "@/math";
import { ZOOM_MAX, ZOOM_MIN } from "@/utils/constants";

export default class Viewport {
  private m_position: vec2;
  private m_zoom: number;
  private m_rotation: number;

  private m_min: vec2;
  private m_max: vec2;
  private m_minZoom: number;
  private m_viewport: vec2;

  constructor({ position, zoom = 1, rotation = 0 }: Partial<ViewportObject>) {
    this.m_position = position ? vec2.clone(position) : vec2.create();
    this.m_zoom = zoom;
    this.m_rotation = rotation;

    this.m_min = [-Infinity, -Infinity];
    this.m_max = [Infinity, Infinity];
    this.m_viewport = vec2.create();
  }

  get position(): vec2 {
    return vec2.clone(this.m_position);
  }

  set position(value: vec2) {
    if (
      vec2.exactEquals(this.m_min, [-Infinity, -Infinity]) &&
      vec2.exactEquals(this.m_max, [Infinity, Infinity])
    ) {
      vec2.copy(this.m_position, value);
      return;
    }

    const minX =
      (this.m_viewport[0] - this.m_max[0] * this.m_zoom) / this.m_zoom;
    const minY =
      (this.m_viewport[1] - this.m_max[1] * this.m_zoom) / this.m_zoom;
    let maxX = this.m_min[0];
    let maxY = this.m_min[1];

    if (this.m_max[0] * this.m_zoom < this.m_viewport[0]) {
      maxX =
        -(this.m_max[0] * this.m_zoom - this.m_viewport[0]) / 2 / this.m_zoom;
    }

    if (this.m_max[1] * this.m_zoom < this.m_viewport[1]) {
      maxY =
        -(this.m_max[1] * this.m_zoom - this.m_viewport[1]) / 2 / this.m_zoom;
    }

    this.m_position = [
      Math.min(Math.max(minX, value[0]), maxX),
      Math.min(Math.max(minY, value[1]), maxY),
    ];
  }

  get zoom(): number {
    return this.m_zoom;
  }

  set zoom(value: number | [number, vec2]) {
    const isArray = Array.isArray(value);
    const zoom = round(
      clamp(
        isArray ? value[0] : value,
        Math.max(ZOOM_MIN, this.m_minZoom),
        ZOOM_MAX
      ),
      4
    );

    if (isArray) {
      const delta = vec2.sub(
        this.clientToLocal(vec2.clone(value[1]), [0, 0], { zoom }),
        this.clientToLocal(vec2.clone(value[1]), [0, 0])
      );

      this.m_zoom = zoom;
      this.position = vec2.add(this.m_position, delta);
    } else this.m_zoom = zoom;
  }

  set viewport(size: vec2) {
    vec2.copy(this.m_viewport, size);
  }

  setBounds(bounds: Box) {
    vec2.copy(this.m_min, bounds[0]);
    vec2.copy(this.m_max, bounds[1]);
    if (bounds[0] > bounds[1])
      this.m_minZoom = this.m_viewport[0] / bounds[1][0];
    else this.m_minZoom = this.m_viewport[1] / bounds[1][1];
  }

  clientToLocal(
    position: vec2,
    offset: vec2,
    override: { position?: vec2; zoom?: number } = {}
  ): vec2 {
    const scene = vec2.create();

    vec2.sub(
      vec2.divS(
        vec2.sub(position, offset, scene),
        override.zoom || this.m_zoom,
        scene
      ),
      override.position || this.m_position,
      scene
    );

    return scene;
  }

  localToClient(
    position: vec2,
    offset: vec2,
    override: { position?: vec2; zoom?: number } = {}
  ) {
    const client = vec2.create();

    vec2.add(
      vec2.mulS(
        vec2.add(position, override.position || this.m_position, client),
        override.zoom || this.m_zoom,
        client
      ),
      offset,
      client
    );

    return client;
  }

  isVisible(box: Box, viewport: vec2): boolean {
    const position = this.m_position;
    const canvasSize = vec2.divS(viewport, this.m_zoom);
    vec2.sub(canvasSize, position, canvasSize);

    return (
      box[1][0] >= -position[0] &&
      box[0][0] <= canvasSize[0] &&
      box[1][1] >= -position[1] &&
      box[0][1] <= canvasSize[1]
    );
  }

  toJSON() {
    return {
      position: this.position,
      zoom: this.m_zoom,
      rotation: this.m_rotation,
    };
  }
}
