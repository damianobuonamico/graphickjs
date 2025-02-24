import getIcon from '@/ui/icons';
import { classNames } from '@/utils/utils';
import { Component, Show } from 'solid-js';

const TextBlock: Component<{
  onChange?(value: string | number): void;
  value: string | number;
  placeholder?: string;
  class?: string;
  leftIcon?: string;
  rightIcon?: string;
  type?: 'text' | 'hex' | 'int' | 'float';
  min?: number;
  max?: number;
}> = (props) => {
  const type = props.type ?? 'text';
  const numeric = type === 'int' || type === 'float';
  const onChange = props.onChange ?? (() => {});

  const validateHex = (value: string) => {
    const hexRe = /^#?([0-9a-f]{3,4}){1,2}$/i;

    console.log(hexRe.test(value));

    if (hexRe.test(value)) {
      if (value.at(0) === '#') return value.substring(1);
      return value;
    }
  };

  const validateNumber = (value: string) => {
    let parsed = type === 'int' ? parseInt(value) : parseFloat(value);

    if (isNaN(parsed)) return undefined;

    if (props.min !== undefined) parsed = Math.max(parsed, props.min);
    if (props.max !== undefined) parsed = Math.min(parsed, props.max);

    return parsed;
  };

  let validateFn: (value: string) => void;

  switch (type) {
    case 'hex':
      validateFn = validateHex;
      break;
    case 'int':
    case 'float':
      validateFn = validateNumber;
      break;
    default:
      validateFn = (value: string) => value;
  }

  return (
    <>
      <Show when={props.leftIcon}>
        <a
          class={classNames('mr-2 text-primary-300', {
            'cursor-ew-resize': numeric
          })}
        >
          {getIcon(props.leftIcon!) ?? props.leftIcon}
        </a>
      </Show>
      <input
        class={classNames(
          'bg-transparent outline-none w-full placeholder-primary-300',
          props.class
        )}
        value={props.value}
        placeholder={props.placeholder}
        type="text"
        onChange={(e: any) => {
          const value = validateFn(e.currentTarget.value);
          if (value !== undefined) {
            console.log('onchange');
            onChange(value);
            e.currentTarget.blur();
          }
        }}
      />
      <Show when={props.rightIcon}>
        <a
          class={classNames('ml-2 text-primary-300', {
            'cursor-ew-resize': numeric
          })}
        >
          {getIcon(props.rightIcon!) ?? props.rightIcon}
        </a>
      </Show>
    </>
  );
};

export default TextBlock;
