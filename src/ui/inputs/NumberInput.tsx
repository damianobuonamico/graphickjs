import { Component, createEffect, createMemo, createSignal } from 'solid-js';
import InputWrapper from './blocks/InputWrapper';
import TextBlock from './blocks/TextBlock';

const NumberInput: Component<{
  onChange?(value: number): void;
  value: number | 'mixed';
  class?: string;
  leftIcon?: string;
  rightIcon?: string;
  type?: 'int' | 'float';
  min?: number;
  max?: number;
  decimals?: number;
}> = (props) => {
  const onChange = props.onChange ?? (() => {});

  const [value, setValue] = createSignal(props.value);

  const displayValue = createMemo(() => (value() === 'mixed' ? '' : value()));
  const placeholder = createMemo(() => (value() === 'mixed' ? 'mixed' : ''));

  createEffect(() => {
    setValue(props.value);
  });

  return (
    <InputWrapper class={props.class}>
      <TextBlock
        onChange={(value: number) => {
          setValue(value);
          onChange(value);
        }}
        value={displayValue()}
        placeholder={placeholder()}
        leftIcon={props.leftIcon}
        rightIcon={props.rightIcon}
        type={props.type ?? 'int'}
        min={props.min}
        max={props.max}
        decimals={props.decimals}
      />
    </InputWrapper>
  );
};

export default NumberInput;
