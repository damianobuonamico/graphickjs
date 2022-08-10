import { JSX, Component, Show, createSignal, createEffect } from 'solid-js';
import { vec2 } from '@math';
import Button, { ButtonVariant } from './Button';
import { ControlledMenu } from '../menu';

interface SelectOption {
  id: string;
  label: string | JSX.Element;
  icon?: JSX.Element;
}

const Select: Component<{
  menuButton: {
    variant: ButtonVariant;
    icon?: JSX.Element;
    arrow?: boolean;
  };
  disabled?: boolean;
  active?: boolean;
  position?: 'right' | 'bottom';
  options: SelectOption[];
  onClick?(current: string): void;
  onChange?(current: string): void;
  useLongPress?: boolean;
}> = (props) => {
  const [active, setActive] = createSignal(false);
  const [anchor, setAnchor] = createSignal(vec2.create());
  const [current, setCurrent] = createSignal(props.options[0]);
  let menuButtonRef: HTMLButtonElement | undefined;

  createEffect(() => {
    if (active() && menuButtonRef) {
      const rect = menuButtonRef.getBoundingClientRect();
      setAnchor(
        props.position === 'right'
          ? vec2.fromValues(rect.x + rect.width + 3, rect.y - 5)
          : vec2.fromValues(rect.x, rect.y + rect.height - 1)
      );
    }
  });

  createEffect(() => {
    if (props.onChange && active()) props.onChange(current().id);
  });

  return (
    <>
      <Button
        variant={props.menuButton.variant}
        disabled={props.disabled}
        onClick={() => {
          if (!props.useLongPress) setActive(true);
          if (props.onClick) props.onClick!(current().id);
        }}
        onLongPress={props.useLongPress ? () => setActive(true) : undefined}
        ref={menuButtonRef}
        leftIcon={props.menuButton.icon}
        active={props.active}
        style={
          props.menuButton.variant === 'file-menu' && current().label === ''
            ? {
                width: '2rem',
                height: '2rem',
                padding: '0',
                'margin-left': '4px',
                'margin-right': '3px'
              }
            : undefined
        }
      >
        {current().label}
        {props.menuButton.arrow && (
          <div class="w-0 h-0 self-end translate-y-[-9px]">
            <svg class="text-primary-500" height="5" width="5">
              <polygon fill={'currentColor'} points="5,0 0,5 5,5" class="triangle" />
            </svg>
          </div>
        )}
      </Button>
      <Show when={active() && !props.disabled}>
        <ControlledMenu
          items={props.options.map((option) => {
            return {
              label: option.label as string,
              callback: () => setCurrent(option),
              active: current().id === option.id
            };
          })}
          anchor={anchor()}
          onClose={() => setActive(false)}
          style={{ padding: '0' }}
          setActiveOnHover={false}
        />
      </Show>
    </>
  );
};

export default Select;
