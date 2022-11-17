interface ComponentCollection {
  stroke?: StrokeComponentCollection;
  fill?: FillComponentCollection;
  background?: GlobalComponent<ColorComponent>;
}

interface PartialComponentCollection {
  stroke?: {
    color?: vec3 | vec4 | string;
    format?: ColorFormat;
    visible?: boolean;
    width?: number;
    corner?: CanvasLineJoin;
  };
  fill?: {
    color?: vec3 | vec4 | string;
    format?: ColorFormat;
    visible?: boolean;
  };
  background?: Partial<ColorComponent>;
}

interface GlobalComponent<T> {
  value: T;
  mixed: boolean;
}
