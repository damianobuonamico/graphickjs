import HistoryManager from '@/editor/history';
import { vec2, vec4 } from '@math';
import colorNames from 'colornames';

class Color implements ColorComponent {
  private m_value: vec4;

  constructor(value: vec4 | string) {
    if (typeof value === 'string') {
      if (!/#/.test(value)) {
        let color = colorNames(value);
        if (!color) color = '#FFFFFF';
        value = color;
      }

      let parsed = value.substring(1);
      if (parsed.length === 3) parsed += 'F';
      else if (parsed.length === 6) parsed += 'FF';

      if (parsed.length === 4) {
        parsed =
          parsed[0] +
          parsed[0] +
          parsed[1] +
          parsed[1] +
          parsed[2] +
          parsed[2] +
          parsed[3] +
          parsed[3];
      }

      this.m_value = [
        parseInt(parsed[0] + parsed[1], 16) / 255,
        parseInt(parsed[2] + parsed[3], 16) / 255,
        parseInt(parsed[4] + parsed[5], 16) / 255,
        parseInt(parsed[6] + parsed[7], 16) / 255
      ];

      return;
    }

    this.m_value = vec4.clone(value);
  }

  public get hex() {
    return (
      '#' +
      (
        (1 << 24) +
        (((this.m_value[0] * 255) << 16) + ((this.m_value[1] * 255) << 8) + this.m_value[2] * 255)
      )
        .toString(16)
        .slice(1)
    );
  }

  public get vec4() {
    return vec4.clone(this.m_value);
  }

  set(color: string) {
    const hex = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(color);
    if (!hex) return;

    const backup: vec3 = [this.m_value[0], this.m_value[1], this.m_value[2]];
    const value: vec3 = [
      parseInt(hex[1], 16) / 255,
      parseInt(hex[2], 16) / 255,
      parseInt(hex[3], 16) / 255
    ];

    HistoryManager.record({
      fn: () => {
        this.m_value[0] = value[0];
        this.m_value[1] = value[1];
        this.m_value[2] = value[2];
      },
      undo: () => {
        this.m_value[0] = backup[0];
        this.m_value[1] = backup[1];
        this.m_value[2] = backup[2];
      }
    });
  }

  public equals(color: vec4 | string | ColorComponent) {
    // TODO: Refactor
    if (color instanceof Color) return vec4.equals(this.m_value, color.m_value);
    return vec4.equals(this.m_value, new Color(color as string).m_value);
  }
}

export default Color;
