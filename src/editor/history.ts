abstract class HistoryManager {
  private static m_undoStack: Action[][] = [];
  private static m_redoStack: Action[][] = [];
  private static m_join = false;

  public static record(actions: Action | Action[]) {
    if (!Array.isArray(actions)) actions = [actions];
    actions.forEach((action) => {
      action.fn();
    });
    if (this.m_join === false) {
      this.m_undoStack.push(actions);
      this.m_redoStack.length = 0;
      this.m_join = true;
      setTimeout(() => {
        this.m_join = false;
      }, 100);
    } else {
      this.m_undoStack[this.m_undoStack.length - 1].push(...actions);
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
    action.forEach((entry) => entry.fn());
    this.m_undoStack.push(action);
    console.timeEnd('redo');
  }
}

export default HistoryManager;
