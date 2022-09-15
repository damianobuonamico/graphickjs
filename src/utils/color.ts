import { vec4 } from '@math';
import colorNames from 'colornames';

class Color {
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

  public get vec4() {
    return vec4.clone(this.m_value);
  }
}

export default Color;
