import { KEYS } from '@/utils/keys';
import CommandHistory from './history/history';
import { Renderer } from './renderer';
import SceneManager from './scene';
import SelectionManager from './selection';
import { save, load } from './data/data';

const actions: { [key: string]: ActionBinding } = {
  undo: {
    callback: () => CommandHistory.undo(),
    shortcut: { ctrl: true, shift: false, key: KEYS.Z }
  },
  redo: {
    callback: () => CommandHistory.redo(),
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
  },
  saveAs: {
    callback: save,
    shortcut: { key: KEYS.S, ctrl: true, shift: true }
  },
  open: {
    callback: load,
    shortcut: { key: KEYS.O, ctrl: true }
  },
  new: {
    callback: () => SceneManager.new(),
    shortcut: { key: KEYS.N, ctrl: true, alt: true }
  }
};

export default actions;
