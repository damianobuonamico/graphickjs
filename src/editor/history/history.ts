import { nanoid } from 'nanoid';
import AnimationManager from '../animation/animation';

abstract class HistoryManager {
  private static m_undoStack: Action[][] = [];
  private static m_redoStack: Action[][] = [];
  private static m_join = false;
  private static m_sequence: Action[] | null = null;
  private static m_skip = false;

  private static push(actions: Action[]) {
    this.m_undoStack.push(actions);
    this.m_redoStack.length = 0;
  }

  public static record(actions: Action | Action[]) {
    if (!Array.isArray(actions)) actions = [actions];

    actions.forEach((action) => {
      action.fn();
    });

    if (this.m_skip) {
      this.m_skip = false;
      return;
    }

    if (this.m_sequence) {
      this.m_sequence.push(...actions);
    } else {
      if (this.m_join === false || !this.m_undoStack.length) {
        this.push(actions);
        this.m_join = true;
        setTimeout(() => {
          this.m_join = false;
        }, 100);
      } else {
        this.m_undoStack[this.m_undoStack.length - 1].push(...actions);
      }
    }
  }

  public static undo() {
    const action = this.m_undoStack.pop();
    if (!action) return;
    console.time('undo');
    action.reverse().forEach((entry) => entry.undo());
    this.m_redoStack.push(action);
    console.timeEnd('undo');
  }

  public static redo() {
    const action = this.m_redoStack.pop();
    if (!action) return;
    console.time('redo');
    action.reverse().forEach((entry) => entry.fn());
    this.m_undoStack.push(action);
    console.timeEnd('redo');
  }

  public static clear() {
    this.m_undoStack.length = 0;
    this.m_redoStack.length = 0;
  }

  public static pop() {
    this.m_undoStack.pop();
  }

  public static beginSequence() {
    this.m_sequence = [];
  }

  public static endSequence() {
    if (this.m_sequence && this.m_sequence.length) this.push(this.m_sequence);
    this.m_sequence = null;
  }

  public static skipNext() {
    this.m_skip = true;
  }

  public static clearSkip() {
    this.m_skip = false;
  }

  static get() {
    return { undo: this.m_undoStack, redo: this.m_redoStack };
  }
}

export default HistoryManager;

class CommandBatch implements GenericCommand {
  readonly id: string = nanoid();

  canMerge: boolean = true;

  private m_commands: GenericCommand[] = [];

  constructor(commands?: GenericCommand[] | GenericCommand) {
    if (!commands) return;
    if (!Array.isArray(commands)) commands = [commands];

    for (let i = 0, n = commands.length; i < n; ++i) {
      this.add(commands[i]);
    }
  }

  get length() {
    return this.m_commands.length;
  }

  get first() {
    return this.m_commands[0];
  }

  add(command: GenericCommand) {
    let merged = false;

    if (command.canMerge) {
      for (let i = 0, n = this.m_commands.length; i < n; ++i) {
        if (this.m_commands[i].canMerge && command.mergeWith(this.m_commands[i])) {
          merged = true;
          break;
        }
      }
    }

    if (!merged) this.m_commands.push(command);
  }

  execute(): void {
    for (let i = 0, n = this.m_commands.length; i < n; ++i) {
      this.m_commands[i].execute();
    }
  }

  undo(): void {
    for (let i = this.m_commands.length - 1; i >= 0; --i) {
      this.m_commands[i].undo();
    }
  }

  mergeWith(batch: CommandBatch): boolean {
    for (let i = 0, n = this.m_commands.length; i < n; ++i) {
      batch.add(this.m_commands[i]);
    }

    return true;
  }
}

export abstract class CommandHistory {
  private static m_commands: GenericCommand[] = [];
  private static m_index: number = -1;

  static add<T>(command: GenericCommand): T {
    const value = command.execute<T>();

    if (this.m_index < this.m_commands.length - 1) {
      for (let i = this.m_commands.length - 1; i > this.m_index; --i) {
        this.m_commands.pop();
      }
    }

    const last = this.m_commands[this.m_index];
    let merged = false;

    if (last && last.canMerge) {
      if (last instanceof CommandBatch) {
        last.add(command);
        merged = true;
      } else if (command.canMerge) {
        merged = command.mergeWith(last);
      }
    }

    if (!merged) {
      this.m_commands.push(new CommandBatch(command));
      this.m_index = this.m_commands.length - 1;
    }

    return value as T;
  }

  static endBatch() {
    if (this.m_index >= 0) {
      let command = this.m_commands[this.m_index];

      if (command instanceof CommandBatch) {
        if (command.length === 0) {
          this.m_commands.splice(this.m_index, 1);
          command = this.m_commands[--this.m_index];
        } else if (command.length === 1) {
          command = this.m_commands[this.m_index] = command.first;
        }
      }

      command.canMerge = false;
    }
  }

  static undo() {
    if (this.m_index >= 0) {
      console.log(this.m_commands);
      console.time('__undo__');
      this.m_commands[this.m_index].undo();
      this.m_index--;
      console.timeEnd('__undo__');
    }
  }

  static redo() {
    const command = this.m_index + 1;
    if (command < this.m_commands.length && command >= 0) {
      console.log(this.m_commands);
      console.time('__redo__');
      this.m_commands[command].execute();
      this.m_index++;
      console.timeEnd('__redo__');
    }
  }

  static clear() {
    this.m_commands.length = 0;
  }
}
