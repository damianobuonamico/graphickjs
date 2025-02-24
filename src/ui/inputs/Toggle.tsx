import { classNames } from '@/utils/utils';
import { Component, createEffect, createSignal, For, JSX } from 'solid-js';
import Button from './Button';

interface ToggleOption {
  id: string | number;
  label: string | JSX.Element;
}

const Toggle: Component<{
  onChange?(id: string | number): void;
  class?: string;
  value: string | number;
  options: ToggleOption[];
  optionsStyle?: JSX.CSSProperties;
}> = (props) => {
  const [value, setValue] = createSignal(props.value);

  createEffect(() => {
    setValue(props.value);
  });

  const toggle = (value: string | number) => {
    if (props.onChange) props.onChange(value);
    setValue(value);
  };

  return (
    <div class={classNames('flex h-input bg-primary-700 w-fit rounded', props.class)}>
      <For each={props.options}>
        {(item) => (
          <Button
            variant="toggle-button"
            style={props.optionsStyle}
            active={item.id === value()}
            onClick={() => toggle(item.id)}
          >
            {item.label}
          </Button>
        )}
      </For>
    </div>
  );
};

export default Toggle;
