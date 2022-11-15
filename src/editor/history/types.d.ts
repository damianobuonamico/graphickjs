interface GenericCommand {
  readonly id: string;

  canMerge: boolean;

  execute<T>(): T | void;
  undo(): void;
  mergeWith(command: GenericCommand): boolean;
}

interface Value<T> {
  value: T;

  add(amount: T): void;
}
