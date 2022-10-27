interface KeyBinding {
  key: string | string[];
  shift?: boolean | null;
  alt?: boolean | null;
  ctrl?: boolean | null;
  platform?: boolean;
}

type ClassNameValue = string | number | boolean | undefined | null;
type ClassNameMapping = { [key: string]: any };
type ClassNameArgument = ClassNameValue | ClassNameMapping | ClassNameArgument[];

interface CacheComponent {
  getCacheAtLevel(level: number): CacheLevelComponent;
  cached<T>(id: string, callback: () => T, level?: number, zoom?: number): T;
  clear(level?: number): void;
}

interface CacheLevelComponent {
  cached<T>(id: string, callback: () => T, zoom?: number): T;
  clear(): void;
  empty(): void;
}
