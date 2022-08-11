import { round } from '@math';

class CanvasStats {
  private m_elements: number = 0;
  private m_entities: number = 0;
  private m_frameStart = 0;
  private m_frameTime = 0;
  private m_frameTimes = 0;
  private m_avgFrameTime = 0;
  private m_max = 0;
  private m_maxIndex = 0;

  private m_lastLogged: CanvasStatsLog | undefined = undefined;

  public get elements() {
    return this.m_elements;
  }

  public get entities() {
    return this.m_entities;
  }

  public get frameTime() {
    return round(this.m_frameTime, 2);
  }

  public get avgFrameTime() {
    return round(this.m_avgFrameTime, 2);
  }

  public get fps() {
    return round(1000 / this.m_frameTime, 2);
  }

  public get avgFps() {
    return round(1000 / this.m_avgFrameTime, 2);
  }

  public get maxFrameTime() {
    return round(this.m_max, 2);
  }

  public get minFps() {
    return round(1000 / this.m_max, 2);
  }

  public clear() {
    this.m_elements = 0;
    this.m_entities = 0;
  }

  public entity() {
    this.m_entities += 1;
  }

  public element() {
    this.m_elements += 1;
    this.m_entities += 1;
  }

  public log() {
    const stats = { entities: this.m_entities, elements: this.m_elements };
    if (
      !this.m_lastLogged ||
      this.m_lastLogged.entities !== stats.entities ||
      this.m_lastLogged.elements !== stats.elements
    ) {
      console.table(stats);
      this.m_lastLogged = stats;
    }
  }

  public begin() {
    this.m_frameStart = performance.now();
  }

  public end() {
    if (this.m_frameTimes - this.m_maxIndex === 1000) this.m_max = 0;
    this.m_frameTime = performance.now() - this.m_frameStart;
    this.m_avgFrameTime =
      (this.m_avgFrameTime * this.m_frameTimes + this.m_frameTime) / (this.m_frameTimes + 1);
    this.m_frameTimes++;
    if (this.m_max < this.m_frameTime) {
      this.m_max = this.m_frameTime;
      this.m_maxIndex = this.m_frameTimes;
    }
  }
}

export default CanvasStats;
