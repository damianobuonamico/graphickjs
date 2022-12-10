import { ChangeCommand } from '@/editor/history/command';
import CommandHistory from '@/editor/history/history';

export class Cache implements CacheComponent {
  private m_map: Map<string, any>;
  private m_genericIds: Map<string, string>;
  private m_zoomMap: Map<string, { zoom: number; value: any }>;
  private m_zoomOffset: number;

  pause: boolean = false;

  constructor() {
    this.m_map = new Map();
    this.m_zoomMap = new Map();
    this.m_genericIds = new Map();
    this.m_zoomOffset = Math.random() / 5;
  }

  public cached<T>(id: string, callback: () => T, genericId?: string): T {
    if (this.pause === true) {
      this.pause = false;
      this.m_map.clear();
      this.m_zoomMap.clear();
    }

    if (this.m_map.has(id)) return this.m_map.get(id);

    const value = callback();
    this.m_map.set(id, value);

    if (genericId) {
      const previousId = this.m_genericIds.get(genericId);
      if (previousId) this.m_map.delete(previousId);
      this.m_genericIds.set(genericId, id);
    }

    return value;
  }

  public cachedZoom<T>(id: string, callback: () => T, zoom: number, tolerance: number = 0.1): T {
    if (this.pause === true) {
      this.pause = false;
      this.m_map.clear();
      this.m_zoomMap.clear();
    }

    // TODO: Check zoom
    let value = this.m_zoomMap.get(id);
    if (value && Math.abs(value.zoom - zoom) < zoom * (tolerance + this.m_zoomOffset)) {
      return value.value;
    }

    value = { zoom, value: callback() };
    this.m_zoomMap.set(id, value);

    return value.value;
  }

  public clear() {
    this.m_map.clear();
  }
}

export class ElementCache {
  private m_caches: [Cache, Cache];

  cached: <T>(id: string, callback: () => T, genericId?: string) => T;
  cachedZoom: <T>(id: string, callback: () => T, zoom: number, tolerance?: number) => T;
  clear: () => void;

  constructor() {
    this.m_caches = [new Cache(), new Cache()];
    this.cached = this.m_caches[0].cached.bind(this.m_caches[0]);
    this.cachedZoom = this.m_caches[0].cachedZoom.bind(this.m_caches[1]);
    this.clear = this.m_caches[0].clear.bind(this.m_caches[0]);
  }

  set pause(value: boolean) {
    this.m_caches[0].pause = value;
    this.m_caches[1].pause = value;
  }

  get last() {
    return this.m_caches[1];
  }
}

export class VertexCache implements CacheComponent {
  private m_caches: [
    CacheComponent | { pause: false; clear: () => void },
    CacheComponent | { pause: false; clear: () => void },
    CacheComponent | { pause: false; clear: () => void }
  ] = [
    { pause: false, clear: () => {} },
    { pause: false, clear: () => {} },
    { pause: false, clear: () => {} }
  ];

  constructor() {}

  set pause(value: boolean) {
    this.m_caches[0].pause = value;
    this.m_caches[1].pause = value;
    this.m_caches[2].pause = value;
  }

  set parentCache(cache: CacheComponent) {
    const backup = this.m_caches[2];
    if (backup === cache) return;

    CommandHistory.add(
      new ChangeCommand(
        () => {
          this.m_caches[2] = cache;
        },
        () => {
          this.m_caches[2] = backup;
        }
      )
    );
  }

  register(cache: CacheComponent) {
    this.m_caches[1] = this.m_caches[0];
    this.m_caches[0] = cache;
  }

  cached<T>(): T {
    return null as T;
  }

  cachedZoom<T>(): T {
    return null as T;
  }

  clear(): void {
    this.m_caches[0].clear();
    this.m_caches[1].clear();
    this.m_caches[2].clear();
  }
}
