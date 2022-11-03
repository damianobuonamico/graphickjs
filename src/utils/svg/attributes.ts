import Fill from '@/editor/ecs/components/fill';
import Stroke from '@/editor/ecs/components/stroke';

class SVGAttributesContainer {
  // Create default attributes
  private m_strokes: { level: number; value: Stroke | null }[] = [{ level: 0, value: null }];
  private m_fills: { level: number; value: Fill | null }[] = [{ level: 0, value: null }];

  public level = 0;

  constructor() {}

  public get stroke() {
    return this.m_strokes[this.m_strokes.length - 1].value?.asObject();
  }

  public get fill() {
    return this.m_fills[this.m_fills.length - 1].value?.asObject();
  }

  private get m_stroke() {
    return this.m_strokes[this.m_strokes.length - 1].value;
  }

  private get m_fill() {
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
      if (!this.m_stroke || !this.m_stroke.color.equals(stroke)) shouldUpdateStroke = true;
    }

    if (fill) {
      if (!this.m_fill || !this.m_fill.color.equals(fill)) shouldUpdateFill = true;
    }

    if (shouldUpdateStroke) {
      if (stroke === 'none') this.m_strokes.push({ level: this.level, value: null });
      else
        this.m_strokes.push({
          level: this.level,
          value: new Stroke({ ...this.m_stroke?.asObject(), ...{ color: stroke }, id: undefined })
        });
    }

    if (shouldUpdateFill) {
      if (fill === 'none') this.m_fills.push({ level: this.level, value: null });
      else
        this.m_fills.push({
          level: this.level,
          value: new Fill({ ...this.m_fill?.asObject(), ...{ color: fill }, id: undefined })
        });
    }
  }
}

export default SVGAttributesContainer;
