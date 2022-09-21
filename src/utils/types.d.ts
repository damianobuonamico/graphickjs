interface KeyBinding {
  key: string | string[];
  shift?: boolean | null;
  alt?: boolean | null;
  ctrl?: boolean | null;
  platform?: boolean;
}

type ClassNameValue = string | number | boolean | undefined | null;
type ClassNameMapping = { [key: string]: any };
type ClassNameArgument =
  | ClassNameValue
  | ClassNameMapping
  | ClassNameArgument[];
