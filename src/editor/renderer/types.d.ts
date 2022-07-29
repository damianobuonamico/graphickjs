interface Canvas {
  container: HTMLDivElement;

  setup(canvas: HTMLCanvasElement): void;
  resize(): void;
}
