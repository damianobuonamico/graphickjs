class CommandBatch implements GenericCommand {
  private m_canMerge: boolean = true;
  private m_commands: GenericCommand[] = [];
  private m_hashes: Map<Object | 'string', number> = new Map();

  constructor(commands?: GenericCommand[] | GenericCommand) {
    if (!commands) return;
    if (!Array.isArray(commands)) commands = [commands];

    for (let i = 0, n = commands.length; i < n; ++i) {
      this.add(commands[i]);
    }
  }

  get canMerge(): boolean {
    return this.m_canMerge;
  }

  set canMerge(canMerge: boolean) {
    if (canMerge === false) this.m_hashes.clear();
    this.m_canMerge = canMerge;
  }

  get hash(): Object {
    return {};
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
      const index = this.m_hashes.get(command.hash);

      if (
        index !== undefined &&
        this.m_commands[index].canMerge &&
        command.mergeWith(this.m_commands[index])
      )
        merged = true;
    }

    if (!merged) {
      this.m_commands.push(command);
      this.m_hashes.set(command.hash, this.m_commands.length - 1);
    }
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

abstract class CommandHistory {
  private static m_commands: GenericCommand[] = [];
  private static m_index: number = -1;
  private static m_ignore: boolean = false;

  static add<T>(command: GenericCommand): T {
    console.trace(command);
    const value = command.execute<T>();
    if (this.m_ignore) return value as T;

    this.seal();

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

  static seal() {
    if (this.m_index < this.m_commands.length - 1) {
      for (let i = this.m_commands.length - 1; i > this.m_index; --i) {
        this.m_commands.pop();
      }
    }
  }

  static pop() {
    if (this.m_commands.pop()) this.m_index--;
  }

  static clear() {
    this.m_commands.length = 0;
    this.m_index = -1;
    console.clear();
  }

  static ignoreNext() {
    this.m_ignore = true;
  }

  static clearIgnore() {
    this.m_ignore = false;
  }
}

export default CommandHistory;
