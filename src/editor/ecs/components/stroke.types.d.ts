interface StrokeComponentCollection {
  color: { value: ColorComponent; mixed: boolean; visible: boolean };
  width: { value: number; mixed: boolean };
  corner: { value: CanvasLineJoin; mixed: boolean };
}

interface StrokeOptions {
  id?: string;
  style?: 'solid' | number[] | null;
  width?: number;
  cap?: CanvasLineCap;
  corner?: CanvasLineJoin;
  miterLimit?: number;
  color?: vec4 | string;
  visible?: boolean;
}

interface StrokeComponentObject {
  id?: string;
  style?: 'solid' | number[] | null;
  width?: number;
  cap?: CanvasLineCap;
  corner?: CanvasLineJoin;
  miterLimit?: number;
  color?: vec4;
  visible?: boolean;
}

interface StrokeComponent {
  id: string;
  style: 'solid' | number[] | null;
  width: number;
  cap: CanvasLineCap;
  corner: CanvasLineJoin;
  miterLimit: number;
  color: ColorComponent;
  visible: boolean;
  tempVisible: boolean;

  asObject(): StrokeComponentObject;
}
