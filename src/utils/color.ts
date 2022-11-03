import { clamp } from '@/math';

export function HSB2RGB([h, s, b]: vec3): vec3 {
  s /= 100;
  b /= 100;

  const k = (n: number) => (n + h / 60) % 6;
  const f = (n: number) => b * (1 - s * Math.max(0, Math.min(k(n), 4 - k(n), 1)));

  return [clamp(f(5), 0, 1), clamp(f(3), 0, 1), clamp(f(1), 0, 1)];
}

export function RGB2HSB([r, g, b]: vec3): vec3 {
  const v = Math.max(r, g, b),
    n = v - Math.min(r, g, b);
  const h = n === 0 ? 0 : n && v === r ? (g - b) / n : v === g ? 2 + (b - r) / n : 4 + (r - g) / n;

  return [60 * (h < 0 ? h + 6 : h), v && (n / v) * 100, v * 100];
}

export function HEX2RGB(hex: string): vec3 | vec4 {
  const parsed = hex.substring(1);
  const size = parsed.length;

  if (size <= 4) {
    if (size === 4)
      return [
        parseInt(parsed[0] + parsed[0], 16) / 255,
        parseInt(parsed[1] + parsed[1], 16) / 255,
        parseInt(parsed[2] + parsed[2], 16) / 255,
        parseInt(parsed[3] + parsed[3], 16) / 255
      ];

    return [
      parseInt(parsed[0] + parsed[0], 16) / 255,
      parseInt(parsed[1] + parsed[1], 16) / 255,
      parseInt(parsed[2] + parsed[2], 16) / 255
    ];
  }

  if (size === 8)
    return [
      parseInt(parsed[0] + parsed[1], 16) / 255,
      parseInt(parsed[2] + parsed[3], 16) / 255,
      parseInt(parsed[4] + parsed[5], 16) / 255,
      parseInt(parsed[6] + parsed[7], 16) / 255
    ];

  return [
    parseInt(parsed[0] + parsed[1], 16) / 255,
    parseInt(parsed[2] + parsed[3], 16) / 255,
    parseInt(parsed[4] + parsed[5], 16) / 255
  ];
}

export function RGB2HEX([r, g, b]: vec3): string {
  return (
    '#' +
    Math.round((1 << 24) + (((r * 255) << 16) + ((g * 255) << 8) + b * 255))
      .toString(16)
      .slice(1)
  );
}
