interface Canvas {
  container: HTMLDivElement;

  setup(canvas: HTMLCanvasElement): void;
  resize(): void;

  beginFrame(): void;
  endFrame(): void;

  clear(...args: any): void;
  rect({
    pos,
    size,
    centered
  }: {
    pos: vec2;
    size: vec2 | number;
    centered: boolean;
  }): void;
}
