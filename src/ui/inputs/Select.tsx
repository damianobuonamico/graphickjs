import { classNames } from '@/utils/utils';
import { Component, For, JSX } from 'solid-js';
import Button from './Button';

interface SelectOption {
  id: string;
  label: string | JSX.Element;
}

const Slider: Component<{
  onChange?(id: string): void;
  class?: string;
  value: string;
  options: SelectOption[];
  optionsStyle?: JSX.CSSProperties;
}> = (props) => {
  console.log(props.value);
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

export default Slider;
