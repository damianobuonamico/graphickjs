import { classNames } from '@/utils/utils';
import { Component, createEffect, createSignal, For, JSX, onMount, Show } from 'solid-js';
import Button from './Button';
import { Popover } from '../overlays';
import getIcon, { CheckIcon } from '../icons';

interface SelectOption {
  id: string | number;
  label: string | JSX.Element;
  icon?: string;
}

const Select: Component<{
  onChange?(id: string | number): void;
  class?: string;
  value: string;
  options: SelectOption[];
  optionsStyle?: JSX.CSSProperties;
}> = (props) => {
  const popoverClose = { fn: () => {} };

  let triggerRef: HTMLButtonElement | undefined;

  const [height, setHeight] = createSignal(0);
  const [deltaY, setDeltaY] = createSignal(0);

  createEffect(() => {
    const i = props.options.findIndex((item) => item.id === props.value);
    setDeltaY(-height() * i);
  });

  onMount(() => {
    if (triggerRef) {
      setHeight(triggerRef.getBoundingClientRect().height);
    }
  });

  return (
    <Popover
      translate={[0, -1 + deltaY()]}
      stopPropagation={true}
      closeFn={popoverClose}
      positioning="over"
    >
      <Button
        ref={triggerRef}
        variant="file-menu"
        class={classNames(
          'border !border-primary-600 hover:!border-primary-500 h-input',
          props.class
        )}
        rightIcon={getIcon('caretDown', { class: 'ml-2' })}
        leftIcon={
          props.options.find((item) => item.id === props.value)?.icon
            ? getIcon(props.options.find((item) => item.id === props.value)?.icon!, {
                class: 'mr-2'
              })
            : undefined
        }
      >
        {props.options.find((item) => item.id === props.value)?.label}
      </Button>

      <For each={props.options}>
        {(item) => (
          <Button
            variant="menu"
            style={props.optionsStyle}
            class="hover:bg-primary-600 w-full"
            active={item.id === props.value}
            leftIcon={
              item.id === props.value ? <CheckIcon /> : item.icon ? getIcon(item.icon) : undefined
            }
            onClick={
              props.onChange
                ? () => {
                    props.onChange!(item.id);
                    popoverClose.fn();
                  }
                : undefined
            }
          >
            {item.label}
          </Button>
        )}
      </For>
    </Popover>
  );
};

export default Select;
