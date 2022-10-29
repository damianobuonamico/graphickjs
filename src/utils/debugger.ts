abstract class Debugger {
  private static m_timers: Map<string, { value: number; entries: number; current: number }> =
    new Map();

  static get timers() {
    return this.m_timers;
  }

  static time(id: string) {
    if (this.m_timers.has(id)) {
      this.m_timers.get(id)!.current = performance.now();
    } else {
      this.m_timers.set(id, { value: 0, entries: 0, current: performance.now() });
    }
  }

  static timeEnd(id: string) {
    const time = performance.now();
    const timer = this.m_timers.get(id);
    if (!timer) return;

    timer.value += time - timer.current;
    timer.entries++;
    timer.current = 0;
  }
}

export default Debugger;
