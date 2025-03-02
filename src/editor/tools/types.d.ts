interface ToolMap<T> {
  select: T;
  selectNode: T;
  selectNodeGroup: T;
  pen: T;
  spline: T;
  text: T;
  line: T;
  arrow: T;
  arc: T;
  spiral: T;
  rectangle: T;
  roundedRectangle: T;
  ellipse: T;
  polygon: T;
  star: T;
  gradient: T;
  eyedropper: T;
  ruler: T;
  pan: T;
  zoom: T;
  scale: T;
  rotate: T;
  pencil: T;
  eraser: T;
}

type Tool = keyof ToolMap<any>;
