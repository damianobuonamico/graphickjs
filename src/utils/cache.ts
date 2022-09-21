class Cache {
  private m_map: Map<string, any>;

  constructor() {
    this.m_map = new Map();
  }

  public cached<T>(id: string, callback: () => T, zoom?: number): T {
    if (this.m_map.has(id) && (!zoom || zoom === this.m_map.get(id + '-zoom')))
      return this.m_map.get(id);
    const value = callback();
    this.m_map.set(id, value);
    if (zoom) this.m_map.set(id + 'zoom', zoom);
    return value;
  }

  public clear() {
    this.m_map.clear();
  }

  public get zoom(): number | undefined {
    return this.m_map.get('zoom');
  }
}

export default Cache;
