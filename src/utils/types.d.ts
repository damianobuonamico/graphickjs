export interface KeyBinding {
  key: string | string[];
  shift?: boolean | null;
  alt?: boolean | null;
  ctrl?: boolean | null;
  platform?: boolean;
}

export type ClassNameValue = string | number | boolean | undefined | null;
export type ClassNameMapping = { [key: string]: any };
export type ClassNameArgument = ClassNameValue | ClassNameMapping | ClassNameArgument[];
