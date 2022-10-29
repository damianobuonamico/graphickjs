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
}

export default HistoryManager;
