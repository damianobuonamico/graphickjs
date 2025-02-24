type Mixed<T> = { [K in keyof T]: T[K] | 'mixed' };
type Operation<T> = Partial<T> | 'add' | 'remove';

interface BackgroundComponentData {
  color: vec4;
}

interface FillComponentData {
  color: vec4;
  rule: number;
  visible: boolean;
}

interface StrokeComponentData {
  color: vec4;
  width: number;
  cap: number;
  join: number;
  miter_limit: number;
  visible: boolean;
}
