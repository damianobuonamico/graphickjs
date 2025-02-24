import { clamp } from '@/math';

export function getWorkspacePrimaryColor(mode: Workspace) {
  switch (mode) {
    case 'whiteboard':
      return '#c867e6';
    case 'publisher':
      return '#ffa666';
    default:
      return '#38c3f2';
  }
}

export function HSV2RGB([h, s, v]: vec3): vec3 {
  h /= 360;
  s /= 100;
  v /= 100;

  let r = 0;
  let g = 0;
  let b = 0;

  const i = Math.floor(h * 6);
  const f = h * 6 - i;
  const p = v * (1 - s);
  const q = v * (1 - f * s);
  const t = v * (1 - (1 - f) * s);

  switch (i % 6) {
    case 0:
      (r = v), (g = t), (b = p);
      break;
    case 1:
      (r = q), (g = v), (b = p);
      break;
    case 2:
      (r = p), (g = v), (b = t);
      break;
    case 3:
      (r = p), (g = q), (b = v);
      break;
    case 4:
      (r = t), (g = p), (b = v);
      break;
    case 5:
      (r = v), (g = p), (b = q);
      break;
  }

  return [r, g, b];
}

export function RGB2HSV([r, g, b]: vec3): vec3 {
  const max = Math.max(r, g, b);
  const min = Math.min(r, g, b);

  let h = 0;
  let s = 0;
  let v = max;

  const d = max - min;

  s = max == 0 ? 0 : d / max;

  if (max != min) {
    switch (max) {
      case r:
        h = (g - b) / d + (g < b ? 6 : 0);
        break;
      case g:
        h = (b - r) / d + 2;
        break;
      case b:
        h = (r - g) / d + 4;
        break;
    }

    h /= 6;
  }

  return [h * 360, s * 100, v * 100];
}

export function HEX2RGB(hex: string): vec3 | vec4 {
  const parsed = hex.at(0) == '#' ? hex.substring(1) : hex;
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
    ((1 << 24) + ((Math.round(r * 255) << 16) + (Math.round(g * 255) << 8) + Math.round(b * 255))) |
    0
  )
    .toString(16)
    .slice(1);
}
