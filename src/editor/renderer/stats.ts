class Stats implements RendererStats {
  private m_beginTime = 0;
  private m_prevTime = 0;
  private m_frames = 0;

  private m_maxSamples = 100;

  ms: number[] = [];
  fps: number[] = [];
  memory: number[] = [];

  minMs: number = Infinity;
  maxMs: number = 0;

  minFps: number = Infinity;
  maxFps: number = 0;

  minMemory: number = Infinity;
  maxMemory: number = 0;

  vertices: number = 0;
  antialiasing: string = 'BROWSER';

  readonly availableMemory: number;
  readonly hasMemoryStats: boolean;

  constructor() {
    const memory = (performance as any).memory;

    this.hasMemoryStats = !!memory;
    this.availableMemory = memory ? Math.round(memory.jsHeapSizeLimit / 1048576) : 0;
  }

  get avgMs() {
    return this.ms.reduce((a, b) => a + b, 0) / this.ms.length;
  }

  get avgFps() {
    return this.fps.reduce((a, b) => a + b, 0) / this.fps.length;
  }

  get avgMemory() {
    return this.memory.reduce((a, b) => a + b, 0) / this.memory.length;
  }

  begin() {
    this.m_beginTime = performance.now();
  }

  end() {
    this.m_frames++;

    const time = performance.now();

    const delta = time - this.m_beginTime;

    this.ms.push(delta);
    this.minMs = Math.min(this.minMs, Math.round(delta));
    this.maxMs = Math.max(this.maxMs, Math.round(delta));

    if (this.ms.length > this.m_maxSamples) this.ms.shift();

    if (time > this.m_prevTime + 1000) {
      const fps = (this.m_frames * 1000) / (time - this.m_prevTime);

      this.minFps = Math.min(this.minFps, Math.round(fps));
      this.maxFps = Math.max(this.maxFps, Math.round(fps));
      this.fps.push(fps);

      if (this.fps.length > this.m_maxSamples) this.fps.shift();

      this.m_prevTime = time;
      this.m_frames = 0;

      if (this.hasMemoryStats) {
        const memory = (performance as any).memory.usedJSHeapSize / 1048576;

        this.memory.push(memory);
        this.minMemory = Math.min(this.minMemory, Math.round(memory));
        this.maxMemory = Math.max(this.maxMemory, Math.round(memory));

        if (this.memory.length > this.m_maxSamples) this.memory.shift();
      }
    }

    return time;
  }
}

export default Stats;
