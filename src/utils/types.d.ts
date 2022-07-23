export interface KeyBinding {
  key: string | string[];
  shift?: boolean | null;
  alt?: boolean | null;
  ctrl?: boolean | null;
  platform?: boolean;
}
