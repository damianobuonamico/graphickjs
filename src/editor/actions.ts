import { KEYS } from '@/utils/keys';
import HistoryManager, { CommandHistory } from './history/history';
import { Renderer } from './renderer';
import SceneManager from './scene';
import SelectionManager from './selection';

const actions: { [key: string]: ActionBinding } = {
  undo: {
    callback: () => {
      // HistoryManager.undo();
      CommandHistory.undo();
    },
    shortcut: { ctrl: true, shift: false, key: KEYS.Z }
  },
  redo: {
    callback: () => {
      // HistoryManager.redo();
      CommandHistory.redo();
    },
    shortcut: { ctrl: true, shift: true, key: KEYS.Z }
  },
  delete: {
    callback: () => SceneManager.delete(true),
    shortcut: { key: KEYS.DELETE }
  },
  toggleStatistics: {
    callback: () => (Renderer.debugging = !Renderer.debugging),
    shortcut: { key: KEYS.S, shift: true, alt: true }
  },
  all: {
    callback: () => SelectionManager.all(),
    shortcut: { key: KEYS.A, ctrl: true }
  },
  invert: {
    callback: () => SelectionManager.invert(),
    shortcut: { key: KEYS.I, ctrl: true, alt: true }
  },
  import: {
    callback: () => SceneManager.import(),
    shortcut: { key: KEYS.I, ctrl: true }
  }
};

export default actions;
