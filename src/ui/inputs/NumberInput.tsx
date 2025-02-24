import { Component } from 'solid-js';
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
}> = (props) => {
  return (
    <InputWrapper class={props.class}>
      <TextBlock
        onChange={props.onChange}
        value={props.value === 'mixed' ? '' : props.value}
        placeholder={props.value === 'mixed' ? 'mixed' : ''}
        leftIcon={props.leftIcon}
        rightIcon={props.rightIcon}
        type={props.type ?? 'int'}
        min={props.min}
        max={props.max}
      />
    </InputWrapper>
  );
};

export default NumberInput;
