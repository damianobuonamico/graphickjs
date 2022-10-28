export class Cache implements CacheComponent {
  private m_map: Map<string, any>;
  private m_genericIds: Map<string, string>;

  pause: boolean = false;

  constructor() {
    this.m_map = new Map();
    this.m_genericIds = new Map();
  }

  public cached<T>(id: string, callback: () => T, genericId?: string): T {
    if (this.pause === true) {
      this.pause = false;
      this.m_map.clear();
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

  public clear() {
    this.m_map.clear();
  }
}

export class ElementCache {
  private m_caches: [Cache, Cache];

  cached: <T>(id: string, callback: () => T, genericId?: string) => T;
  clear: () => void;

  constructor() {
    this.m_caches = [new Cache(), new Cache()];
    this.cached = this.m_caches[0].cached.bind(this.m_caches[0]);
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

export class VertexCache {
  private m_caches: [Cache | { pause: false }, Cache | { pause: false }, Cache | { pause: false }] =
    [{ pause: false }, { pause: false }, { pause: false }];

  constructor() {}

  set pause(value: boolean) {
    this.m_caches[0].pause = value;
    this.m_caches[1].pause = value;
    this.m_caches[2].pause = value;
  }

  set parentCache(cache: Cache) {
    this.m_caches[2] = cache;
  }

  register(cache: Cache) {
    this.m_caches[1] = this.m_caches[0];
    this.m_caches[0] = cache;
  }
}
