interface CacheComponent {
  pause: boolean;

  cached<T>(id: string, callback: () => T, genericId?: string): T;
  clear(): void;
}
