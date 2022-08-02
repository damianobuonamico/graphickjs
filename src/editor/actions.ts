import { KEYS } from '@/utils/keys';
import HistoryManager from './history';

const actions: { [key: string]: ActionBinding } = {
  undo: {
    callback: HistoryManager.undo.bind(HistoryManager),
    shortcut: { ctrl: true, shift: false, key: KEYS.Z }
  },
  redo: {
    callback: HistoryManager.redo.bind(HistoryManager),
    shortcut: { ctrl: true, shift: true, key: KEYS.Z }
  }
};

export default actions;
