interface Callback {
  callback(): boolean;
  level: number;
}

export default class MenuKeyCallback {
  private m_callbacks: Callback[] = [];
  private m_flushed = false;

  constructor() {}

  private flush() {
    if (this.m_flushed) return;
    this.m_flushed = true;
    this.m_callbacks.sort((a, b) => (a.level < b.level ? 1 : -1));
    for (const callback of this.m_callbacks) if (callback.callback()) break;
    this.m_callbacks.length = 0;
    setTimeout(() => {
      this.m_flushed = false;
    }, 25);
  }

  public register(callback: () => boolean, level: number) {
    this.m_callbacks.push({ callback, level });
    setTimeout(() => {
      this.flush();
    }, 25);
  }
}
