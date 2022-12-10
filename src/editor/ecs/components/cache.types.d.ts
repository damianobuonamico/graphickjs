interface CacheComponent {
  pause: boolean;

  cached<T>(id: string, callback: () => T, genericId?: string): T;
  cachedZoom<T>(id: string, callback: () => T, zoom: number): T;

  clear(): void;
}
