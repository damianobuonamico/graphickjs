import { vec2, vec4 } from '@/math';

export class Command implements GenericCommand {
  canMerge: boolean = true;

  constructor() {}

  get hash(): Object {
    return {};
  }

  execute(): void {}

  undo(): void {}

  mergeWith(command: GenericCommand): boolean {
    return false;
  }
}

export class ChangeCommand<T> extends Command {
  constructor(execute: () => T, undo: () => void) {
    super();

    this.execute = execute;
    this.undo = undo;
  }
}

// TODO: ChangePrimitivePairCommand

export class FunctionCallCommand<T> extends Command {
  canMerge: false = false;

  constructor(fn: () => T) {
    super();

    this.execute = fn;
    this.undo = fn;
  }
}

export class ChangePrimitiveCommand<T> extends Command {
  private m_value: { value: T };
  private m_oldValue: T;
  private m_newValue: T;

  constructor(oldValue: { value: T }, newValue: T) {
    super();

    this.m_value = oldValue;
    this.m_oldValue = oldValue.value;
    this.m_newValue = newValue;
  }

  get hash(): Object {
    return this.m_value;
  }

  execute(): void {
    this.m_value.value = this.m_newValue;
  }

  undo(): void {
    this.m_value.value = this.m_oldValue;
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof ChangePrimitiveCommand && command.m_value === this.m_value) {
      command.m_newValue = this.m_newValue;
      return true;
    }

    return false;
  }
}

export class ChangeVec2Command extends Command {
  private m_value: vec2;
  private m_oldValue: vec2;
  private m_newValue: vec2;

  constructor(oldValue: vec2, newValue: vec2) {
    super();

    this.m_value = oldValue;
    this.m_oldValue = vec2.clone(oldValue);
    this.m_newValue = vec2.clone(newValue);
  }

  get hash(): Object {
    return this.m_value;
  }

  execute(): void {
    vec2.copy(this.m_value, this.m_newValue);
  }

  undo(): void {
    vec2.copy(this.m_value, this.m_oldValue);
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof ChangeVec2Command && command.m_value === this.m_value) {
      command.m_newValue = this.m_newValue;
      return true;
    }

    return false;
  }
}

export class ChangeVec4Command extends Command {
  private m_value: vec4;
  private m_oldValue: vec4;
  private m_newValue: vec4;

  constructor(oldValue: vec4, newValue: vec4) {
    super();

    this.m_value = oldValue;
    this.m_oldValue = vec4.clone(oldValue);
    this.m_newValue = vec4.clone(newValue);
  }

  get hash(): Object {
    return this.m_value;
  }

  execute(): void {
    vec4.copy(this.m_value, this.m_newValue);
  }

  undo(): void {
    vec4.copy(this.m_value, this.m_oldValue);
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof ChangeVec4Command && command.m_value === this.m_value) {
      command.m_newValue = this.m_newValue;
      return true;
    }

    return false;
  }
}

export class SetToMapCommand<K, V> extends Command {
  private m_map: MapSuper<K, V>;
  private m_keys: K[];
  private m_values: V[];

  constructor(map: MapSuper<K, V>, key: K, value: V) {
    super();

    this.m_map = map;
    this.m_keys = [key];
    this.m_values = [value];
  }

  get hash(): Object {
    return this.m_map;
  }

  execute(): void {
    for (let i = 0, n = this.m_keys.length; i < n; ++i) {
      this.m_map.superSet(this.m_keys[i], this.m_values[i]);
    }
  }

  undo(): void {
    for (let i = this.m_keys.length - 1; i > -1; --i) {
      this.m_map.superDelete(this.m_keys[i]);
    }
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof SetToMapCommand && command.m_map === this.m_map) {
      command.m_keys.push(...this.m_keys);
      command.m_values.push(...this.m_values);
      return true;
    }

    return false;
  }
}

export class DeleteFromMapCommand<K, V> extends Command {
  private m_map: MapSuper<K, V>;
  private m_keys: K[];
  private m_values: V[];

  constructor(map: MapSuper<K, V>, key: K, value: V) {
    super();

    this.m_map = map;
    this.m_keys = [key];
    this.m_values = [value];
  }

  get hash(): Object {
    return this.m_map;
  }

  execute(): void {
    for (let i = 0, n = this.m_keys.length; i < n; ++i) {
      this.m_map.superDelete(this.m_keys[i]);
    }
  }

  undo(): void {
    for (let i = this.m_keys.length - 1; i > -1; --i) {
      this.m_map.superSet(this.m_keys[i], this.m_values[i]);
    }
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof DeleteFromMapCommand && command.m_map === this.m_map) {
      command.m_keys.push(...this.m_keys);
      command.m_values.push(...this.m_values);
      return true;
    }

    return false;
  }
}

export class SetToOrderedMapCommand<K, V> extends Command {
  private m_map: OrderedMapSuper<K, V>;
  private m_keys: K[];
  private m_values: V[];
  private m_indices: (number | undefined)[];

  constructor(map: OrderedMapSuper<K, V>, key: K, value: V, index?: number) {
    super();

    this.m_map = map;
    this.m_keys = [key];
    this.m_values = [value];
    this.m_indices = [index];
  }

  get hash(): Object {
    return this.m_map;
  }

  execute(): void {
    for (let i = 0, n = this.m_keys.length; i < n; ++i) {
      const index = this.m_indices[i];
      if (index) this.m_map.order.splice(index, 0, this.m_keys[i]);
      else this.m_map.order.push(this.m_keys[i]);

      this.m_map.superSet(this.m_keys[i], this.m_values[i]);
    }
  }

  undo(): void {
    for (let i = this.m_keys.length - 1; i > -1; --i) {
      const index = this.m_indices[i];
      if (index) this.m_map.order.splice(index, 1);
      else this.m_map.order.pop();

      this.m_map.superDelete(this.m_keys[i]);
    }
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof SetToOrderedMapCommand && command.m_map === this.m_map) {
      command.m_keys.push(...this.m_keys);
      command.m_values.push(...this.m_values);
      command.m_indices.push(...this.m_indices);
      return true;
    }

    return false;
  }
}

export class DeleteFromOrderedMapCommand<K, V> extends Command {
  private m_map: OrderedMapSuper<K, V>;
  private m_keys: K[];
  private m_values: V[];
  private m_indices: number[];

  constructor(map: OrderedMapSuper<K, V>, key: K, value: V, index: number) {
    super();

    this.m_map = map;
    this.m_keys = [key];
    this.m_values = [value];
    this.m_indices = [index];
  }

  get hash(): Object {
    return this.m_map;
  }

  execute(): void {
    for (let i = 0, n = this.m_keys.length; i < n; ++i) {
      this.m_map.order.splice(this.m_indices[i], 1);
      this.m_map.superDelete(this.m_keys[i]);
    }
  }

  undo(): void {
    for (let i = this.m_keys.length - 1; i > -1; --i) {
      this.m_map.order.splice(this.m_indices[i], 0, this.m_keys[i]);
      this.m_map.superSet(this.m_keys[i], this.m_values[i]);
    }
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof DeleteFromOrderedMapCommand && command.m_map === this.m_map) {
      command.m_keys.push(...this.m_keys);
      command.m_values.push(...this.m_values);
      command.m_indices.push(...this.m_indices);
      return true;
    }

    return false;
  }
}

export class PauseCacheCommand extends Command {
  private m_caches: CacheComponent[];

  constructor(value: CacheComponent) {
    super();

    this.m_caches = [value];
  }

  get hash(): Object {
    return 'cache';
  }

  execute(): void {
    for (let i = 0, n = this.m_caches.length; i < n; ++i) {
      this.m_caches[i].pause = true;
    }
  }

  undo(): void {
    for (let i = this.m_caches.length - 1; i > -1; --i) {
      this.m_caches[i].pause = true;
    }
  }

  mergeWith(command: GenericCommand): boolean {
    if (command instanceof PauseCacheCommand) {
      command.m_caches.push(...this.m_caches);
      return true;
    }
    return false;
  }
}
