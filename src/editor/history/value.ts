import { vec2 } from '@/math';
import { ChangeCommand, ChangeVec2Command } from './command';
import { CommandHistory } from './history';

export class Vec2Value implements Value<vec2> {
  private m_value: vec2;

  constructor(value?: vec2) {
    this.m_value = value ? vec2.clone(value) : vec2.create();
  }

  get value(): vec2 {
    return vec2.clone(this.m_value);
  }

  set value(value: vec2) {
    CommandHistory.add(new ChangeVec2Command(this.m_value, value));
  }

  add(amount: vec2): void {
    CommandHistory.add(new ChangeVec2Command(this.m_value, vec2.add(this.m_value, amount)));
  }
}

export class FloatValue implements Value<number> {
  constructor(value?: number) {}
}

export class StringValue implements Value<string> {
  constructor(value?: string) {}
}

export class MapValue<K, V> extends Map<K, V> {
  constructor(entries?: readonly (readonly [K, V])[] | null) {
    super(entries);
  }

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

    return CommandHistory.add(
      new ChangeCommand(
        () => {
          return super.delete(key);
        },
        () => {
          super.set(key, value);
        }
      )
    );
  }

  set(key: K, value: V): this {
    return CommandHistory.add(
      new ChangeCommand(
        () => {
          return super.set(key, value);
        },
        () => {
          super.delete(key);
        }
      )
    );
  }
}
