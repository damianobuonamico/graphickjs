import CanvasBackend2D from '../renderer/2D/backend2d';

class Sequencer extends CanvasBackend2D {
  private m_shouldUpdate = false;

  constructor(canvas: HTMLCanvasElement) {
    super();
    this.setup(canvas);
  }

  resize() {
    super.resize();

    this.m_shouldUpdate = true;
    this.render();
  }

  onPointerDown() {
    console.log('down');
  }

  onPointerMove() {
    console.log('move');
  }

  onPointerUp() {
    console.log('up');
  }

  render() {
    if (!this.m_shouldUpdate) return;

    this.clear({ color: '#0E1117' });
    this.debugRect({ position: [10, 10] });
    this.m_ctx.strokeStyle = '#FFF';

    this.draw({
      operations: [
        { type: 'beginPath' },
        { type: 'moveTo', data: [[Math.random() * 20, 20]] },
        { type: 'lineTo', data: [[100, 50]] },
        { type: 'stroke' },
        { type: 'closePath' }
      ]
    });

    this.m_shouldUpdate = false;
  }
}

export default Sequencer;
