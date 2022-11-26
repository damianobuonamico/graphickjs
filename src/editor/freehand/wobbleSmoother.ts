import { vec2 } from '@/math';
import { normalize01 } from './utils';

abstract class WobbleSmoother {
  private static m_samples: Sample[];
  private static m_weightedPositionSum: vec2 = [0, 0];
  private static m_distanceSum: number = 0;
  private static m_durationSum: number = 0;
  private static m_params: WobbleSmootherParams;

  static reset(params: WobbleSmootherParams, position: vec2, time: number): void {
    this.m_params = params;
    this.m_samples = [{ position, time, weightedPosition: [0, 0], distance: 0, duration: 0 }];
    this.m_weightedPositionSum = [0, 0];
    this.m_distanceSum = 0;
    this.m_durationSum = 0;
  }

  static update(position: vec2, time: number): vec2 {
    const back = this.m_samples[this.m_samples.length - 1];
    const deltaTime = time - back.time;

    this.m_samples.push({
      position,
      weightedPosition: vec2.mulS(position, deltaTime),
      distance: vec2.dist(position, back.position),
      duration: deltaTime,
      time
    });

    vec2.add(this.m_weightedPositionSum, back.weightedPosition, this.m_weightedPositionSum);
    this.m_distanceSum += back.distance;
    this.m_durationSum += back.duration;

    while (this.m_samples[0].time < time - this.m_params.timeout) {
      const front = this.m_samples[0];

      vec2.sub(this.m_weightedPositionSum, front.weightedPosition, this.m_weightedPositionSum);
      this.m_distanceSum -= front.distance;
      this.m_durationSum -= front.duration;

      this.m_samples.shift();
    }

    if (this.m_durationSum == 0) return position;

    const avgPosition = vec2.divS(this.m_weightedPositionSum, this.m_durationSum);
    const avgSpeed = this.m_distanceSum / this.m_durationSum;

    return vec2.lerp(
      avgPosition,
      position,
      normalize01(this.m_params.speedFloor, this.m_params.speedCeiling, avgSpeed)
    );
  }
}

export default WobbleSmoother;
