import { KEYS } from './keys';

export function classNames(...args: ClassNameArgument[]) {
  const classes: string[] = [];

  for (let i = 0; i < args.length; i++) {
    const arg = arguments[i];
    if (!arg) continue;

    const argType = typeof arg;

    if (argType === 'string' || argType === 'number') {
      classes.push(arg);
    } else if (Array.isArray(arg)) {
      if (arg.length) {
        if (arg[arg.length - 1] === false) continue;
        const inner = classNames.apply(null, arg);
        if (inner) {
          classes.push(inner);
        }
      }
    } else if (argType === 'object') {
      if (arg.toString === Object.prototype.toString) {
        for (const key in arg) {
          if (arg.hasOwnProperty(key) && arg[key]) {
            classes.push(key);
          }
        }
      } else {
        classes.push(arg.toString());
      }
    }
  }

  return classes.join(' ');
}

export function isObject(value: any) {
  if (
    !(typeof value === 'object' && value !== null) ||
    Object.prototype.toString.call(value) != '[object Object]'
  ) {
    return false;
  }
  if (Object.getPrototypeOf(value) === null) {
    return true;
  }
  let proto = value;
  while (Object.getPrototypeOf(proto) !== null) {
    proto = Object.getPrototypeOf(proto);
  }
  return Object.getPrototypeOf(value) === proto;
}

export function cloneObject(object: { [key: string]: any }) {
  const obj: { [key: string]: any } = {};
  Object.entries(object).forEach(([key, value]) => {
    const type = typeof value;
    if (type !== 'object' || value === null) {
      obj[key] = value;
    } else {
      obj[key] === cloneObject(value);
    }
  });
  return obj;
}

export function fillObject<T>(object: { [key: string]: any }, schema: { [key: string]: any }) {
  const incomplete = cloneObject(object);

  Object.entries(schema).forEach(([key, value]) => {
    if (!incomplete[key]) {
      if (isObject(value)) incomplete[key] = cloneObject(value);
      else incomplete[key] = value;
    } else if (isObject(incomplete[key])) incomplete[key] = fillObject(incomplete[key], value);
  });

  return incomplete as T;
}

export function isInputLike(
  target: Element | EventTarget | null
): target is HTMLInputElement | HTMLTextAreaElement | HTMLSelectElement | HTMLSpanElement {
  return (
    (target instanceof HTMLElement && target.dataset.type === 'wysiwyg') ||
    target instanceof HTMLInputElement ||
    target instanceof HTMLTextAreaElement ||
    target instanceof HTMLSelectElement ||
    target instanceof HTMLSpanElement
  );
}

export function isShortcut(e: KeyboardEvent, shortcut: KeyBinding): boolean {
  if (e.key.toLowerCase() !== shortcut.key) return false;
  if (!!shortcut.ctrl !== (e as any)[KEYS.CTRL]) return false;
  if (!!shortcut.shift !== e.shiftKey) return false;
  if (!!shortcut.alt !== e.altKey) return false;
  return true;
}
