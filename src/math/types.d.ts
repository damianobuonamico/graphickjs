interface IndexedCollection extends Iterable<number> {
  readonly length: number;
  [index: number]: number;
}

declare type vec2 = [number, number] | IndexedCollection;
