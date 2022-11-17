type ColorFormat = 'rgb' | 'hsb' | 'hex';

interface ColorComponent {
  readonly hex: string;
  readonly hsb: vec3;
  readonly vec4: vec4;

  alpha: number;

  parse(color: string | vec3 | vec4, format?: ColorFormat): vec3 | vec4 | null;
  set(color: string | vec3 | vec4, format?: ColorFormat): void;
  toString(): string;

  equals(color: vec4 | string | ColorComponent): boolean;
}
