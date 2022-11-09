import { round } from '@/math';
import CanvasBackend2D from './renderer/2D/backend2d';

abstract class AnimationManager {
  private static m_playing = false;
  private static m_smoothInteractions = true;
  private static m_fps = 24;
  private static m_fpsInterval = 1000 / this.m_fps;
  private static m_lastDrawTime = performance.now();
  private static m_lastSampleTime = this.m_lastDrawTime;
  private static m_frameCount = 0;
  private static m_sampleFrequency = 1000;
  private static m_intervalId: ReturnType<typeof setInterval>;
  private static m_requestId: number;
  private static m_currentFps = this.m_fps;
  private static m_started = false;

  private static m_canvas: CanvasBackend2D;

  private static m_renderFn: () => void;

  static set canvas(canvas: HTMLCanvasElement) {
    this.m_canvas = new CanvasBackend2D();
    this.m_canvas.setup(canvas);
  }

  static resize() {
    this.m_canvas.resize();
    this.m_canvas.debugRect({ position: [10, 10] });
    this.m_canvas.draw({
      operations: [
        { type: 'beginPath' },
        { type: 'moveTo', data: [[50, 50]] },
        { type: 'lineTo', data: [[100, 50]] },
        { type: 'stroke' },
        { type: 'closePath' }
      ]
    });
  }

  static get playing(): boolean {
    return this.m_playing;
  }

  static get fps(): number {
    return round(this.m_currentFps, 2);
  }

  static set renderFn(fn: () => void) {
    this.m_renderFn = fn;
  }

  static play() {
    this.m_playing = true;
    this.render();
  }

  static pause() {
    this.m_playing = false;
  }

  private static startAnimating() {
    this.m_started = true;
    this.m_lastDrawTime = performance.now();
    this.m_lastSampleTime = this.m_lastDrawTime;
    this.m_frameCount = 0;

    this.animate(0);

    this.m_intervalId = setInterval(this.sampleFps.bind(this), this.m_sampleFrequency);
  }

  private static stopAnimating() {
    this.m_started = false;
    clearInterval(this.m_intervalId);
  }

  private static animate(now: number) {
    if (!this.m_playing) {
      this.stopAnimating();
      return;
    }

    this.m_requestId = requestAnimationFrame(this.animate.bind(this));

    let elapsed = now - this.m_lastDrawTime;

    if (elapsed > this.m_fpsInterval) {
      this.m_lastDrawTime = now - (elapsed % this.m_fpsInterval);

      this.m_renderFn();

      this.m_frameCount++;
    }
  }

  static render() {
    if (!this.m_playing || this.m_smoothInteractions) {
      requestAnimationFrame(this.m_renderFn);
    } else if (!this.m_started) {
      this.startAnimating();
    }
  }

  private static sampleFps() {
    let now = performance.now();

    if (this.m_frameCount > 0) {
      let currentFps = (this.m_frameCount / (now - this.m_lastSampleTime)) * 1000;
      this.m_currentFps = currentFps;

      this.m_frameCount = 0;
    }

    this.m_lastSampleTime = now;
  }
}

export default AnimationManager;
