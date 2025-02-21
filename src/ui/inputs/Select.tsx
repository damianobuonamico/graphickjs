import { classNames } from '@/utils/utils';
import { Component, For, JSX } from 'solid-js';
import Button from './Button';

interface SelectOption {
  id: string;
  label: string | JSX.Element;
}

const Select: Component<{
  onChange?(id: string): void;
  class?: string;
  value: string;
  options: SelectOption[];
  optionsStyle?: JSX.CSSProperties;
}> = (props) => {
  return (
    <div class={classNames('flex', props.class)}>
      <For each={props.options}>
        {(item) => (
          <Button
            variant="select-button"
            style={props.optionsStyle}
            active={item.id === props.value}
            onClick={props.onChange ? () => props.onChange!(item.id) : undefined}
          >
            {item.label}
          </Button>
        )}
      </For>
    </div>
  );
};

export default Select;
