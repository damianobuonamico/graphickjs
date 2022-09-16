import Fill from '@/editor/ecs/components/fill';
import Stroke from '@/editor/ecs/components/stroke';

class SVGAttributesContainer {
  // Create default attributes
  private m_strokes: { level: number; value: Stroke | null }[] = [{ level: 0, value: null }];
  private m_fills: { level: number; value: Fill | null }[] = [{ level: 0, value: null }];

  public level = 0;

  constructor() {}

  public get stroke() {
    return this.m_strokes[this.m_strokes.length - 1].value;
  }

  public get fill() {
    return this.m_fills[this.m_fills.length - 1].value;
  }

  get(node: SVGElement) {
    while (this.m_strokes[this.m_strokes.length - 1].level >= this.level) {
      this.m_strokes.pop();
    }

    while (this.m_fills[this.m_fills.length - 1].level >= this.level) {
      this.m_fills.pop();
    }

    const stroke = node.getAttribute('stroke') || undefined;
    const fill = node.getAttribute('fill') || undefined;

    let shouldUpdateStroke = false;
    let shouldUpdateFill = false;

    if (stroke) {
      if (!this.stroke || !this.stroke.color.equals(stroke)) shouldUpdateStroke = true;
    }

    if (fill) {
      if (!this.fill || !this.fill.color.equals(fill)) shouldUpdateFill = true;
    }

    console.log(shouldUpdateFill);

    if (shouldUpdateStroke) {
      if (stroke === 'none') this.m_strokes.push({ level: this.level, value: null });
      else
        this.m_strokes.push({
          level: this.level,
          value: new Stroke({ ...this.stroke?.toJSON(), ...{ color: stroke }, id: undefined })
        });
    }

    if (shouldUpdateFill) {
      if (fill === 'none') this.m_fills.push({ level: this.level, value: null });
      else
        this.m_fills.push({
          level: this.level,
          value: new Fill({ ...this.fill?.toJSON(), ...{ color: fill }, id: undefined })
        });
    }

    console.log(this.fill?.id);
  }
}

export default SVGAttributesContainer;
