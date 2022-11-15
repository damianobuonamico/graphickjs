import { vec2 } from '@/math';
import { nanoid } from 'nanoid';

export class Command implements GenericCommand {
  readonly id = nanoid();

  canMerge: boolean = true;

  constructor() {}

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
