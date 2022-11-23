import { round } from '@/math';
import Sequencer from './sequencer/sequencer';

abstract class AnimationManager {
  private static m_playing = false;
  private static m_smoothInteractions = true;
  private static m_fps = 60;
  private static m_fpsInterval = 1000 / this.m_fps;
  private static m_lastDrawTime = performance.now();
  private static m_frameCount = 0;
  private static m_requestId: number;
  private static m_currentFps = this.m_fps;
  private static m_started = false;

  private static m_sequencer: Sequencer | undefined;

  private static m_renderFn: () => void;

  static set canvas(canvas: HTMLCanvasElement) {
    this.m_sequencer = new Sequencer(canvas);
  }

  static resize() {
    this.m_sequencer?.resize();
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

  static stop() {
    this.m_playing = false;
    this.m_sequencer?.stop();
    this.render();
  }

  private static startAnimating() {
    this.m_started = true;
    this.m_lastDrawTime = performance.now();
    this.m_frameCount = 0;

    this.animate(0);
  }

  private static stopAnimating() {
    this.m_started = false;
  }

  static add(entity: Entity) {
    this.m_sequencer?.add(entity);
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

    this.m_sequencer?.animate(this.m_fpsInterval);
  }

  static render() {
    if (!this.m_playing) {
      requestAnimationFrame(this.m_renderFn);
    } else if (!this.m_started) {
      this.startAnimating();
    } else if (this.m_smoothInteractions && this.m_fps < 60) {
      requestAnimationFrame(this.m_renderFn);
    }
  }

  static renderSequencer() {
    requestAnimationFrame(() => this.m_sequencer?.render());
  }

  static onPointerDown() {
    this.m_sequencer?.onPointerDown();
  }

  static onPointerMove() {
    this.m_sequencer?.onPointerMove();
  }

  static onPointerUp() {
    this.m_sequencer?.onPointerUp();
  }

  static onWheel(e: WheelEvent) {
    this.m_sequencer?.onWheel(e);
  }

  static toJSON() {
    return this.m_sequencer?.toJSON();
  }

  static load(sequence: Entity[]) {
    if (!this.m_sequencer) return;
    setTimeout(() => {
      this.m_sequencer?.load(sequence);
    }, 100);
  }
}

export default AnimationManager;
