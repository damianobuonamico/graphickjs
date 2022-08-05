import { KEYS } from '@/utils/keys';
import HistoryManager from './history';
import SceneManager from './scene';

const actions: { [key: string]: ActionBinding } = {
  undo: {
    callback: HistoryManager.undo.bind(HistoryManager),
    shortcut: { ctrl: true, shift: false, key: KEYS.Z }
  },
  redo: {
    callback: HistoryManager.redo.bind(HistoryManager),
    shortcut: { ctrl: true, shift: true, key: KEYS.Z }
  },
  delete: {
    callback: () => SceneManager.delete(true),
    shortcut: { key: KEYS.DELETE }
  }
};

export default actions;
