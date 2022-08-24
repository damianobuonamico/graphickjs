import { round } from '@math';

class CanvasStats {
  private m_entities: number = 0;
  private m_drawn: number = 0;

  private m_beginTime = 0;
  private m_prevTime = 0;
  private m_frames = 0;
  private m_samples = 0;

  private m_renderTime = 0;
  private m_avgRenderTime = 0;
  private m_minRenderTime = Infinity;
  private m_maxRenderTime = -Infinity;

  private m_fps = 0;
  private m_avg = 0;
  private m_min = Infinity;
  private m_max = -Infinity;

  private m_memory = 0;
  private m_maxMemory = 0;

  constructor() {
    this.m_maxMemory = (performance as any).memory
      ? round((performance as any).memory.jsHeapSizeLimit / 1048576, 2)
      : 0;
  }

  public get entities() {
    return this.m_entities;
  }

  public get drawn() {
    return this.m_drawn;
  }

  public get ren() {
    return this.m_renderTime;
  }

  public get fps() {
    return [this.m_fps, this.m_renderTime];
  }

  public get avg() {
    return [this.m_avg, this.m_avgRenderTime];
  }

  public get min() {
    return [this.m_min, this.m_maxRenderTime];
  }

  public get max() {
    return [this.m_max, this.m_minRenderTime];
  }

  public get mem() {
    return this.m_memory;
  }

  public get heap() {
    return this.m_maxMemory;
  }

  public clear() {
    this.m_entities = 0;
    this.m_drawn = 0;
  }

  public entity() {
    this.m_entities++;
  }

  public draw() {
    this.m_drawn++;
  }

  public getColor(value: number) {
    if (value > 49) return 'springgreen';
    else if (value > 20) return 'orange';
    return 'crimson';
  }

  public begin() {
    this.m_beginTime = (performance || Date).now();
  }

  public end() {
    const time = (performance || Date).now();
    this.m_frames++;

    if (this.m_samples > 100) {
      this.m_samples = 0;
      this.m_max = 0;
      this.m_min = 1000;
    }

    if (time > this.m_prevTime + 200) {
      this.m_renderTime = round(time - this.m_beginTime, 2);
      this.m_fps = round(1000 / this.m_renderTime, 2);
      this.m_memory = (performance as any).memory
        ? round((performance as any).memory.usedJSHeapSize / 1048576, 2)
        : 0;

      this.m_max = Math.max(this.m_max, this.m_fps);
      this.m_min = Math.min(this.m_min, this.m_fps);
      this.m_maxRenderTime = Math.max(this.m_maxRenderTime, this.m_renderTime);
      this.m_minRenderTime = Math.min(this.m_minRenderTime, this.m_renderTime);
      this.m_avg = round((this.m_avg * this.m_samples + this.m_fps) / (this.m_samples + 1), 2);
      this.m_avgRenderTime = round(
        (this.m_avgRenderTime * this.m_samples + this.m_renderTime) / (this.m_samples + 1),
        2
      );

      this.m_prevTime = time;
      this.m_frames = 0;
      this.m_samples++;
    }
  }
}

export default CanvasStats;
