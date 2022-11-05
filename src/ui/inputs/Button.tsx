import { classNames } from '@utils/utils';
import { JSX, Component, Show } from 'solid-js';
import getIcon from '../icons';

export type ButtonVariant =
  | 'button'
  | 'select-button'
  | 'file-menu'
  | 'file-menu-icon'
  | 'menu'
  | 'menu-icon'
  | 'tool';

const Button: Component<{
  children: string | number | JSX.Element;
  variant?: ButtonVariant;
  onClick?(e: MouseEvent): void;
  onLongPress?(e: MouseEvent): void;
  onMouseDown?(e: MouseEvent): void;
  onMouseUp?(e: MouseEvent): void;
  onHover?(): void;
  onLeave?(): void;
  lighter?: boolean;
  active?: boolean;
  ref?: HTMLButtonElement;
  leftIcon?: JSX.Element;
  rightIcon?: JSX.Element;
  disabled?: boolean;
  style?: JSX.CSSProperties;
}> = (props) => {
  const variant = props.variant || 'button';
  const isIcon = variant.includes('icon');
  const isFile = variant.includes('file');
  const isMenu = variant.includes('menu');
  const isButton = variant.includes('button');
  let longPressTimer: number;

  const onMouseDown = (e: MouseEvent) => {
    if (props.onMouseDown) props.onMouseDown(e);
    longPressTimer = window.setTimeout(() => {
      props.onLongPress!(e);
    }, 300);
  };

  const onMouseUp = (e: MouseEvent) => {
    if (props.onMouseUp) props.onMouseUp(e);
    clearTimeout(longPressTimer);
  };

  return (
    <button
      ref={props.ref}
      onClick={props.onClick}
      onMouseDown={props.onLongPress ? onMouseDown : props.onMouseDown}
      onMouseUp={props.onLongPress ? onMouseUp : props.onMouseUp}
      onMouseOver={props.onHover}
      onMouseLeave={props.onLeave}
      style={props.style}
      class={classNames(
        'flex items-center outline-none',
        {
          'cursor-default': props.disabled,
          'bg-primary-700': props.active && !props.disabled,
          'text-primary-500': props.disabled
        },
        [
          [
            'py-1 grow',
            { 'grid justify-items-start grid-cols-menu-item': !isIcon },
            { 'grid justify-items-start grid-cols-menu-item': isIcon },
            !isFile
          ],
          [
            'rounded h-fi border',
            {
              'hover:bg-primary-700': !props.disabled,
              'border-primary-600': props.active && !props.disabled,
              'border-transparent': !props.active || props.disabled
            },
            { 'px-2 py-0.5': !isIcon },
            { 'w-[24px] h-[24px] items-center justify-center': isIcon },
            isFile
          ],
          isMenu
        ],
        [
          'flex items-center justify-center w-8 h-8 m-1 rounded',
          { 'hover:text-primary': !props.active },
          { 'bg-primary-700 text-primary': props.active },
          variant === 'tool'
        ],
        [
          'aspect-square w-8 h-8 rounded flex items-center justify-center hover:bg-primary-600',
          { 'text-primary': props.active && props.variant !== 'select-button' },
          isButton
        ]
      )}
    >
      <Show when={variant === 'menu' || props.leftIcon}>
        <div class="flex items-center justify-center h-full w-full">
          {typeof props.leftIcon === 'string' ? getIcon(props.leftIcon) : props.leftIcon}
        </div>
      </Show>
      {props.children}
      <Show when={variant === 'menu' || props.rightIcon}>
        <div class="flex items-center justify-center h-full w-full">
          {typeof props.rightIcon === 'string' ? getIcon(props.rightIcon) : props.rightIcon}
        </div>
      </Show>
    </button>
  );
};

export default Button;
