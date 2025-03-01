import { round } from '@/math';
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
  decimals?: number;
}> = (props) => {
  const type = props.type ?? 'text';
  const numeric = type === 'int' || type === 'float';
  const onChange = props.onChange ?? (() => {});

  let onPointerDownValue: number = 0;
  let onPointerDownOrigin: number = 0;

  const validateHex = (value: string) => {
    const hexRe = /^#?([0-9a-f]{3,4}){1,2}$/i;

    if (hexRe.test(value)) {
      if (value.at(0) === '#') return value.substring(1);
      return value;
    }
  };

  const validateNumeric = (value: number) => {
    if (typeof value !== 'number' || isNaN(value)) return undefined;

    if (type === 'int') value = Math.round(value);
    else value = parseFloat(value.toFixed(props.decimals ?? 3));

    if (props.min !== undefined) value = Math.max(value, props.min);
    if (props.max !== undefined) value = Math.min(value, props.max);

    if (value === props.value) return undefined;

    return value;
  };

  const validateNumber = (value: string) => {
    return validateNumeric(type === 'int' ? parseInt(value) : parseFloat(value));
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

  const onPointerDown = async (e: PointerEvent) => {
    document.addEventListener('pointermove', onPointerMove);
    document.addEventListener('pointerup', onPointerUp);

    onPointerDownValue = props.value as number;
    onPointerDownOrigin = e.clientX;
  };

  const onPointerMove = async (e: PointerEvent) => {
    // try {
    //   await e.target.requestPointerLock({
    //     unadjustedMovement: true
    //   });
    // } catch (error) {
    //   if (error.name === 'NotSupportedError') {
    //     // Some platforms may not support unadjusted movement.
    //     await e.target.requestPointerLock();
    //   } else {
    //     throw error;
    //   }
    // }

    let diff = (e.clientX - onPointerDownOrigin) * (e.shiftKey ? 0.1 : 1);
    let value = validateNumeric(onPointerDownValue + diff);

    if (value !== undefined) {
      onChange(value);
    }
  };

  const onPointerUp = () => {
    document.removeEventListener('pointermove', onPointerMove);
    document.removeEventListener('pointerup', onPointerUp);

    // try {
    //   document.exitPointerLock();
    // } catch (error) {
    //   // Do nothing
    // }
  };

  return (
    <div class="flex">
      <Show when={props.leftIcon}>
        <a
          onPointerDown={numeric ? onPointerDown : undefined}
          class={classNames('mr-2 text-primary-300 font-semibold', {
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
        value={
          numeric && typeof props.value === 'number'
            ? round(props.value as number, props.decimals ?? 3)
            : props.value
        }
        placeholder={props.placeholder}
        type="text"
        onChange={(e: any) => {
          const value = validateFn(e.currentTarget.value);
          if (value !== undefined) {
            onChange(value);
            e.currentTarget.blur();
          }
        }}
        onKeyDown={
          numeric
            ? (e: KeyboardEvent) => {
                if (e.key === 'ArrowUp') {
                  const value = validateNumeric((props.value as number) + 1);
                  if (value !== undefined) onChange(value);
                } else if (e.key === 'ArrowDown') {
                  const value = validateNumeric((props.value as number) - 1);
                  if (value !== undefined) onChange(value);
                }
              }
            : undefined
        }
      />
      <Show when={props.rightIcon}>
        <a
          onPointerDown={numeric ? onPointerDown : undefined}
          class={classNames('ml-2 text-primary-300 font-bold', {
            'cursor-ew-resize': numeric
          })}
        >
          {getIcon(props.rightIcon!) ?? props.rightIcon}
        </a>
      </Show>
    </div>
  );
};

export default TextBlock;
