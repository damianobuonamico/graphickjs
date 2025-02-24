import { Component } from 'solid-js';
import { IconProps } from './types';

const CapSquareIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M13 3L12.5 3L2.5 3L2 3V3.5V7.5V11.5V12H2.5H12.5H13V11.5V3.57V3ZM12 4V11H3V8H8.5C8.77614 8 9 7.77613 9 7.5C9 7.22385 8.77614 7 8.5 7H3V4L12 4Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CapSquareIcon;
