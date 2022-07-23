import { ClassNameArgument } from './types';

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
