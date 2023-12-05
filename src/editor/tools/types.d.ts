interface ToolMap<T> {
  select: T;
  directSelect: T;
  pen: T;
  rectangle: T;
  ellipse: T;
  pan: T;
  zoom: T;
  scale: T;
  rotate: T;
  pencil: T;
  eraser: T;
}

type Tool = keyof ToolMap<any>;
