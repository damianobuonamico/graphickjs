declare module '*.glsl' {
  const value: string;
  export default value;
}

interface OffscreenCanvas extends HTMLCanvasElement {
  new (width: number, height: number): HTMLCanvasElement;
}
