import { lerp, vec2 } from '@/math';
import Freehand from '../ecs/entities/freehand';
import StrokeModeler from '../freehand/strokeModeler';
import WobbleSmoother from '../freehand/wobbleSmoother';
import InputManager from '../input';
import SceneManager from '../scene';

const onPencilPointerDown = () => {
  StrokeModeler.reset({
    wobbleSmootherParams: { timeout: 40, speedFloor: 1.31, speedCeiling: 1.44 },
    positionModelerParams: { springMassConstant: 11 / 32400, dragConstant: 72 },
    samplingParams: {
      minOutputRate: -1,
      endOfStrokeStoppingDistance: -1,
      endOfStrokeMaxIterations: 20,
      maxOutputsPerCall: 100000
    },
    stylusStateModelerParams: {
      maxInputSamples: 10
    },
    predictionParams: {
      processNoise: -1,
      measurementNoise: -1,
      minStableIteration: 4,
      maxTimeSamples: 20,
      minCatchupVelocity: -1,
      accelerationWeight: 0.5,
      jerkWeight: 0.1,
      predictionInterval: -1,
      confidenceParams: {
        desiredNumberOfSamples: 20,
        maxEstimationDistance: -1,
        minTravelSpeed: -1,
        maxTravelSpeed: -1,
        maxLinearDeviation: -1,
        baselineLinearityConfidence: 0.4
      }
    },
    experimentalParams: {}
  });

  const results: Result[] = [];

  // TODO: tilt
  StrokeModeler.update(
    {
      eventType: 'down',
      position: [0, 0],
      time: InputManager.time,
      pressure: InputManager.pressure || 0.5,
      tilt: Math.PI / 2,
      orientation: 0
    },
    results
  );

  const wobbleSmoother = new WobbleSmoother();
  wobbleSmoother.reset(
    { timeout: 40, speedFloor: 1.31, speedCeiling: 1.44 },
    [0, 0],
    InputManager.time
  );

  const unprocessedFreehand = new Freehand({
    position: InputManager.scene.position,
    points: [[0, 0, InputManager.pressure || 0.5]]
  });

  const stabilizedFreehand = new Freehand({
    position: InputManager.scene.position,
    points: [[0, 0, InputManager.pressure || 0.5]],
    color: '#333'
  });

  const processedFreehand = new Freehand({
    position: InputManager.scene.position,
    points: [[0, 0, InputManager.pressure || 0.5]],
    color: '#444'
  });

  // SceneManager.add(unprocessedFreehand);
  SceneManager.add(stabilizedFreehand);
  SceneManager.add(processedFreehand);

  function onPointerMove() {
    unprocessedFreehand.add(InputManager.scene.delta, InputManager.pressure);

    const correctedPosition = wobbleSmoother.update(InputManager.scene.delta, InputManager.time);
    stabilizedFreehand.add(correctedPosition, InputManager.pressure);

    StrokeModeler.update(
      {
        eventType: 'move',
        position: InputManager.scene.delta,
        time: InputManager.time,
        pressure: InputManager.pressure || 0.5,
        tilt: Math.PI / 2,
        orientation: 0
      },
      results
    );

    // StrokeModeler.predict(results);
    processedFreehand.points = results.map((result) => [...result.position, result.pressure]);
  }

  function onPointerUp() {
    StrokeModeler.update(
      {
        eventType: 'up',
        position: InputManager.scene.delta,
        time: InputManager.time,
        pressure: InputManager.pressure || 0.5,
        tilt: Math.PI / 2,
        orientation: 0
      },
      results
    );

    // StrokeModeler.predict(results);
    processedFreehand.points = results.map((result) => [...result.position, result.pressure]);
  }

  return {
    onPointerMove,
    onPointerUp
  };
};

export default onPencilPointerDown;
