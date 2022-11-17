interface GenericCommand {
  canMerge: boolean;

  readonly hash: Object;

  execute<T>(): T | void;
  undo(): void;
  mergeWith(command: GenericCommand): boolean;
}

interface Value<T> {
  value: T;

  add(amount: T): void;
}

interface MapSuper<K, V> extends Map<K, V> {
  superSet(key: K, value: V): Map<K, V>;
  superDelete(key: K): boolean;
}
