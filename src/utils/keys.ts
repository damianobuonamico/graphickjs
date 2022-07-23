import { KeyBinding } from './types';

export const isDarwin = /Mac|iPod|iPhone|iPad/.test(window.navigator.platform);
export const isWindows = /^Win/.test(window.navigator.platform);

export const KEYS = {
  ALT: 'Alt',
  ARROW_LEFT: 'ArrowLeft',
  ARROW_RIGHT: 'ArrowRight',
  ARROW_DOWN: 'ArrowDown',
  ARROW_UP: 'ArrowUp',
  ESCAPE: 'Escape',
  DELETE: 'Delete',
  BACKSPACE: 'Backspace',
  SPACEBAR: ' ',
  META: isDarwin ? 'metaKey' : 'ctrlKey',
  META_KEY: isDarwin ? 'Meta' : 'Control',
  PLATFORM: isDarwin ? 'ctrlKey' : 'metaKey',
  PLATFORM_KEY: isDarwin ? 'Control' : 'Meta',
  SHIFT: 'Shift',
  ENTER: 'Enter',
  A: 'a',
  B: 'b',
  C: 'c',
  D: 'd',
  E: 'e',
  F: 'f',
  G: 'g',
  H: 'h',
  I: 'i',
  J: 'j',
  K: 'k',
  L: 'l',
  M: 'm',
  N: 'n',
  O: 'o',
  P: 'p',
  Q: 'q',
  R: 'r',
  S: 's',
  T: 't',
  U: 'u',
  V: 'v',
  W: 'w',
  X: 'x',
  Y: 'y',
  Z: 'z'
};

export const BUTTONS = {
  TOUCH: -1,
  LEFT: 0,
  MIDDLE: 1,
  RIGHT: 2
};

export function getShortcutString(shortcut: KeyBinding): string {
  let string = '';
  if (shortcut.ctrl) string += isDarwin ? 'Cmd+' : 'Ctrl+';
  if (shortcut.platform) string += isDarwin ? 'Ctrl+' : 'Win+';
  if (shortcut.shift) string += 'Shift+';
  if (shortcut.alt) string += 'Alt+';
  return (string += Array.isArray(shortcut.key)
    ? shortcut.key[0].toUpperCase()
    : shortcut.key.toUpperCase());
}
