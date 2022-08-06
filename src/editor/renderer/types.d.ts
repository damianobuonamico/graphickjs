interface Canvas {
  container: HTMLDivElement;
  DOM: HTMLCanvasElement;

  size: vec2;
  offset: vec2;

  setup(canvas: HTMLCanvasElement): void;
  resize(): void;

  beginFrame(): void;
  endFrame(): void;

  clear(...args: any): void;
  rect({
    pos,
    size,
    centered,
    color,
    transform
  }: {
    pos: vec2;
    size: vec2 | number;
    centered: boolean;
    color: vec4;
    transform?: mat4;
  }): void;
}
