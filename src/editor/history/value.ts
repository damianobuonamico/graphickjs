import { vec2, vec4 } from '@/math';
import {
  ChangeCommand,
  ChangePrimitiveCommand,
  ChangeVec2Command,
  ChangeVec4Command,
  DeleteFromMapCommand,
  DeleteFromOrderedMapCommand,
  FunctionCallCommand,
  SetToMapCommand,
  SetToOrderedMapCommand
} from './command';
import CommandHistory from './history';

export class BooleanValue implements Value<boolean> {
  protected m_value: { value: boolean };

  constructor(value?: boolean) {
    this.m_value = { value: !!value };
  }

  get value(): boolean {
    return this.m_value.value;
  }

  set value(value: boolean) {
    CommandHistory.add(new ChangePrimitiveCommand(this.m_value, value));
  }

  add(): void {
    this.value = !this.m_value.value;
  }
}

export class StringValue implements Value<string> {
  protected m_value: { value: string };

  constructor(value?: string) {
    this.m_value = { value: value || '' };
  }

  get value(): string {
    return this.m_value.value;
  }

  set value(value: string) {
    CommandHistory.add(new ChangePrimitiveCommand(this.m_value, value));
  }

  add(string: string): void {
    CommandHistory.add(new ChangePrimitiveCommand(this.m_value, this.m_value.value + string));
  }
}

export class FloatValue implements Value<number> {
  protected m_value: { value: number };

  constructor(value?: number) {
    this.m_value = { value: value || 0 };
  }

  get value(): number {
    return this.m_value.value;
  }

  set value(value: number) {
    if (this.m_value.value === value) return;
    CommandHistory.add(new ChangePrimitiveCommand(this.m_value, value));
  }

  add(amount: number): void {
    if (amount === 0) return;
    CommandHistory.add(new ChangePrimitiveCommand(this.m_value, this.m_value.value + amount));
  }
}

export class Vec2Value implements Value<vec2> {
  protected m_value: vec2;

  constructor(value?: vec2) {
    this.m_value = value ? vec2.clone(value) : vec2.create();
  }

  get value(): vec2 {
    return vec2.clone(this.m_value);
  }

  set value(value: vec2) {
    if (vec2.exactEquals(this.m_value, value)) return;
    CommandHistory.add(new ChangeVec2Command(this.m_value, value));
  }

  add(amount: vec2): void {
    if (amount[0] === 0 && amount[1] === 0) return;
    CommandHistory.add(new ChangeVec2Command(this.m_value, vec2.add(this.m_value, amount)));
  }
}

export class Vec4Value implements Value<vec4> {
  protected m_value: vec4;

  constructor(value?: vec4) {
    this.m_value = value ? vec4.clone(value) : vec4.create();
  }

  get value(): vec4 {
    return vec4.clone(this.m_value);
  }

  set value(value: vec4) {
    if (vec4.exactEquals(this.m_value, value)) return;
    CommandHistory.add(new ChangeVec4Command(this.m_value, value));
  }

  add(amount: vec4): void {
    if (amount[0] === 0 && amount[1] === 0 && amount[2] === 0 && amount[3] === 0) return;
    CommandHistory.add(new ChangeVec4Command(this.m_value, vec4.add(this.m_value, amount)));
  }
}

export class MapValue<K, V> extends Map<K, V> {
  constructor(entries?: readonly (readonly [K, V])[] | null) {
    super(entries);
  }

  superDelete = super.delete;
  superSet = super.set;

  clear(): void {
    const entries = this.entries();

    CommandHistory.add(
      new ChangeCommand(
        () => {
          return super.clear();
        },
        () => {
          for (const entry of entries) super.set(entry[0], entry[1]);
        }
      )
    );
  }

  delete(key: K): boolean {
    const value = this.get(key);
    if (!value) return false;

    CommandHistory.add(new DeleteFromMapCommand(this, key, value));
    return true;
  }

  set(key: K, value: V): this {
    if (this.has(key)) {
      const oldValue = this.get(key);

      if (oldValue === value) return this;
      else this.delete(key);
    }

    CommandHistory.add(new SetToMapCommand(this, key, value));
    return this;
  }
}

export class OrderedMapValue<K, V> extends Map<K, V> {
  private m_order: K[];
  private m_setCallback: ((value: V, key: K) => void) | undefined;
  private m_deleteCallback: ((value: V, key: K) => void) | undefined;

  constructor(
    entries?: readonly (readonly [K, V])[] | null,
    setCallback?: (value: V, key: K) => void,
    deleteCallback?: (value: V, key: K) => void
  ) {
    super(entries);
    this.m_order = entries ? entries.map((entry) => entry[0]) : [];
    this.m_setCallback = setCallback;
    this.m_deleteCallback = deleteCallback;
  }

  get first(): V | undefined {
    return this.get(this.m_order[0]);
  }

  get last(): V | undefined {
    return this.get(this.m_order[this.m_order.length - 1]);
  }

  get order(): K[] {
    return this.m_order;
  }

  superDelete = super.delete;
  superSet = super.set;

  clear(): void {
    const keys = [...this.m_order];
    const entries = this.entries();

    CommandHistory.add(
      new ChangeCommand(
        () => {
          this.m_order.length = 0;
          return super.clear();
        },
        () => {
          this.m_order = [...keys];
          for (const entry of entries) super.set(entry[0], entry[1]);
        }
      )
    );
  }

  set(key: K, value: V, index?: number): this {
    if (this.has(key)) {
      const oldValue = this.get(key);

      if (oldValue === value) return this;
      else this.delete(key);
    }

    CommandHistory.add(new SetToOrderedMapCommand(this, key, value, index, this.m_setCallback));

    return this;
  }

  delete(key: K): boolean {
    const value = this.get(key);
    if (!value) return false;
    const index = this.m_order.indexOf(key);

    CommandHistory.add(
      new DeleteFromOrderedMapCommand(this, key, value, index, this.m_deleteCallback)
    );

    return true;
  }

  forEach(
    callbackfn: (value: V, key: K, map: OrderedMapValue<K, V>, index: number) => void,
    thisArg?: any
  ): void {
    if (thisArg) callbackfn = callbackfn.bind(thisArg);

    for (let i = 0, n = this.m_order.length; i < n; ++i) {
      callbackfn(this.get(this.m_order[i])!, this.m_order[i], this, i);
    }
  }

  forEachReversed(
    callbackfn: (value: V, key: K, map: OrderedMapValue<K, V>, index: number) => void,
    thisArg?: any
  ): void {
    if (thisArg) callbackfn = callbackfn.bind(thisArg);

    for (let i = this.m_order.length - 1; i > -1; --i) {
      callbackfn(this.get(this.m_order[i])!, this.m_order[i], this, i);
    }
  }

  map<T>(callbackfn: (value: V, id: K, index: number, array: K[]) => T, thisArg?: any): T[] {
    if (thisArg) callbackfn = callbackfn.bind(thisArg);

    return this.m_order
      .filter((key) => this.has(key))
      .map((key, index, array) => callbackfn(this.get(key)!, key, index, array));
  }

  reorder(order: K[], callbackfn?: (order: K[]) => void): K[] {
    const backup = [...this.m_order];
    const reordered = order.filter((key) => this.has(key));

    let isSameOrder = true;

    if (backup.length === reordered.length) {
      for (let i = 0, n = backup.length; i < n; ++i) {
        if (backup[i] !== reordered[i]) {
          isSameOrder = false;
          break;
        }
      }
    } else isSameOrder = false;

    if (!isSameOrder) {
      return CommandHistory.add(
        new ChangeCommand(
          () => {
            this.m_order = [...reordered];
            if (callbackfn) callbackfn(reordered);
            return reordered;
          },
          () => {
            this.m_order = [...backup];
            if (callbackfn) callbackfn(backup);
          }
        )
      );
    } else if (callbackfn) {
      return CommandHistory.add(
        new ChangeCommand(
          () => {
            callbackfn(reordered);
            return reordered;
          },
          () => {
            callbackfn(backup);
          }
        )
      );
    }

    return this.m_order;
  }

  reverse(): K[] {
    return CommandHistory.add(new FunctionCallCommand(this.m_order.reverse.bind(this.m_order)));
  }

  indexOf(searchElement: K, fromIndex?: number): number {
    return this.m_order.indexOf(searchElement, fromIndex);
  }

  slice(start?: number, end?: number): K[] {
    return this.m_order.slice(start, end);
  }
}

export class EntityValue<T extends Entity> implements Value<T | undefined> {
  protected m_value: { value: T | undefined };

  constructor(value?: T) {
    this.m_value = { value };
  }

  get value(): T | undefined {
    return this.m_value.value;
  }

  set value(value: T | undefined) {
    if (this.m_value.value === value) return;
    CommandHistory.add(new ChangePrimitiveCommand(this.m_value, value));
  }

  add(): void {}
}
