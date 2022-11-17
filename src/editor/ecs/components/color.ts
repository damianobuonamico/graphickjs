import { Vec4Value } from '@/editor/history/value';
import { HEX2RGB, HSB2RGB, RGB2HEX, RGB2HSB } from '@/utils/color';
import { vec4 } from '@math';
import * as colorNames from 'color-name';

class Color extends Vec4Value implements ColorComponent {
  constructor(value: vec3 | vec4 | string) {
    super();

    const parsed = this.parse(value);

    if (!parsed) vec4.copy(this.m_value, [1, 1, 1, 1]);
    else if (parsed.length === 3) vec4.copy(this.m_value, [...parsed, 1]);
    else vec4.copy(this.m_value, parsed);
  }

  get alpha() {
    return this.m_value[3];
  }

  set alpha(value: number) {
    this.value = [this.m_value[0], this.m_value[1], this.m_value[2], value];
  }

  get hex() {
    return RGB2HEX([this.m_value[0], this.m_value[1], this.m_value[2]]);
  }

  get hsb(): vec3 {
    return RGB2HSB([this.m_value[0], this.m_value[1], this.m_value[2]]);
  }

  get vec4() {
    return vec4.clone(this.m_value);
  }

  toString() {
    return `rgba(${this.m_value[0] * 255}, ${this.m_value[1] * 255}, ${this.m_value[2] * 255}, ${
      this.m_value[3]
    })`;
  }

  parse(color: string | vec3 | vec4, format: ColorFormat = 'rgb'): vec3 | vec4 | null {
    if (Array.isArray(color)) {
      switch (format) {
        case 'rgb': {
          if (color.length === 4)
            return [
              color[0] > 1 ? color[0] / 255 : color[0],
              color[1] > 1 ? color[1] / 255 : color[1],
              color[2] > 1 ? color[2] / 255 : color[2],
              color[3] > 1 ? color[3] / 100 : color[3]
            ];

          return [
            color[0] > 1 ? color[0] / 255 : color[0],
            color[1] > 1 ? color[1] / 255 : color[1],
            color[2] > 1 ? color[2] / 255 : color[2]
          ];
        }
        case 'hsb': {
          const hsb = HSB2RGB([color[0], color[1], color[2]]);

          if (color.length === 3) return hsb;

          return [...hsb, color[3] > 1 ? color[3] / 100 : color[3]];
        }
      }

      return null;
    }

    color = color.trim();

    if (color === 'transparent') return [0, 0, 0, 0];
    if (format === 'hex' || color[0] === '#') return HEX2RGB(color);

    const parsed = /^((?:rgb|hsb)a?)\s*\(([^\)]*)\)/.exec(color);

    if (parsed) {
      format = parsed[1] as ColorFormat;

      const values = parsed[2]
        .trim()
        .split(/\s*[,\/]\s*|\s+/)
        .map((v) => parseFloat(v));

      if (values.length < 3) return null;

      const hasAlpha = values.length > 3;

      if (format[format.length - 1] === 'a')
        format = format.substring(0, format.length - 1) as ColorFormat;

      switch (format) {
        case 'rgb': {
          if (hasAlpha)
            return [
              values[0] > 1 ? values[0] / 255 : values[0],
              values[1] > 1 ? values[1] / 255 : values[1],
              values[2] > 1 ? values[2] / 255 : values[2],
              values[3] > 1 ? values[3] / 100 : values[3]
            ];

          return [
            values[0] > 1 ? values[0] / 255 : values[0],
            values[1] > 1 ? values[1] / 255 : values[1],
            values[2] > 1 ? values[2] / 255 : values[2]
          ];
        }
        case 'hsb': {
          const hsb = HSB2RGB([
            values[0] < 1 ? values[0] * 360 : values[0],
            values[1] < 1 ? values[1] * 100 : values[1],
            values[2] < 1 ? values[2] * 100 : values[2]
          ]);

          if (color.length === 3) return hsb;

          return [...hsb, values[3] <= 1 ? values[3] : values[3] / 100];
        }
      }
    }

    const rgbValues = (colorNames as any)[color];
    if (!rgbValues) return null;

    return rgbValues.map((v: number) => v / 255);
  }

  set(color: string, format?: ColorFormat) {
    const parsed = this.parse(color, format);
    if (!parsed) return;

    if (parsed.length === 3) this.value = [...parsed, this.m_value[3]];
    else this.value = parsed;
  }

  equals(color: vec4 | string | ColorComponent): boolean {
    if (color instanceof Color) return vec4.equals(this.m_value, color.m_value);
    else if (Array.isArray(color)) return vec4.equals(this.m_value, color);

    const parsed = this.parse(color as string);
    if (!parsed) return false;

    return vec4.equals(this.m_value, parsed.length === 3 ? [...parsed, 1] : parsed);
  }
}

export default Color;
