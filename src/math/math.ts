export function clamp(t: number, min: number, max: number) {
  return Math.min(Math.max(min, t), max);
}

export function round(t: number, decimals = 0) {
  decimals = 10 ** decimals;
  return Math.round((t + Number.EPSILON) * decimals) / decimals;
}
