// This class constructs a prediction using the same PositionModeler class as
// the SpringBasedModeler, fixing the anchor position and allowing the stroke to
// "catch up". The way the prediction is constructed is very similar to how the

import PositionModeler from '../positionModeler';

// SpringBasedModeler models the end of a stroke.
class StrokeEndPredictor implements InputPredictor {
  private m_lastPosition: vec2 | null;
  private m_positionModelerParams: PositionModelerParams;
  private m_samplingParams: SamplingParams;

  constructor(positionModelerParams: PositionModelerParams, samplingParams: SamplingParams) {
    this.m_positionModelerParams = positionModelerParams;
    this.m_samplingParams = samplingParams;
  }

  reset(): void {
    this.m_lastPosition = null;
  }

  update(position: vec2, time: number): void {
    this.m_lastPosition = position;
  }

  constructPrediction(last_state: TipState, prediction: TipState[]): void {
    prediction.length = 0;
    if (!this.m_lastPosition) {
      // We don't yet have enough data to construct a prediction.
      return;
    }

    const modeler = new PositionModeler();
    modeler.reset(last_state, this.m_positionModelerParams);
    modeler.modelEndOfStroke(
      this.m_lastPosition,
      1 / this.m_samplingParams.minOutputRate,
      this.m_samplingParams.endOfStrokeMaxIterations,
      this.m_samplingParams.endOfStrokeStoppingDistance,
      prediction
    );
  }
}

export default StrokeEndPredictor;
