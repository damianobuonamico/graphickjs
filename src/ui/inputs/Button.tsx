import { JSX, Component, Show } from 'solid-js';
import classNames from 'classnames';

export type ButtonVariant = 'file-menu' | 'menu';

const Button: Component<{
  children: string | number;
  variant?: ButtonVariant;
  onClick?(): void;
  onMouseDown?(): void;
  onMouseUp?(): void;
  onHover?(): void;
  onLeave?(): void;
  lighter?: boolean;
  active?: boolean;
  ref?: HTMLButtonElement;
  leftIcon?: JSX.Element;
  rightIcon?: JSX.Element;
}> = (props) => {
  const variant = props.variant || 'file-menu';

  return (
    <button
      ref={props.ref}
      onClick={props.onClick}
      onMouseDown={props.onMouseDown}
      onMouseUp={props.onMouseUp}
      onMouseOver={props.onHover}
      onMouseLeave={props.onLeave}
      // TODO: create custom implementation of classNames()
      class={classNames(
        'select-none flex items-center outline-none',
        {
          'py-1 grow grid justify-items-start grid-cols-menu-item': variant === 'menu',
          'bg-primary-700': props.active
        },
        {
          'px-2 rounded h-fi border': variant === 'file-menu',
          'border-primary-600': props.active,
          'border-transparent': !props.active,
          'hover:bg-primary-700': variant === 'file-menu' && !props.lighter,
          'hover:bg-primary-600': variant === 'file-menu' && props.lighter
        }
      )}
    >
      <Show when={variant === 'menu' || props.leftIcon}>
        <div class="flex items-center justify-center h-full w-full">{props.leftIcon}</div>
      </Show>
      {props.children}
      <Show when={variant === 'menu' || props.rightIcon}>
        <div class="flex items-center justify-center h-full w-full">{props.rightIcon}</div>
      </Show>
    </button>
  );
};

export default Button;
