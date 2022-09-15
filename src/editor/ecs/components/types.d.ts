interface StrokeOptions {
  id?: string;
  style?: 'solid' | number[] | null;
  width?: number;
  cap?: CanvasLineCap;
  corner?: CanvasLineJoin;
  miterLimit?: number;
  color?: vec4 | string;
}

interface FillOptions {
  id?: string;
  style?: 'solid';
  color?: vec4 | string;
}
