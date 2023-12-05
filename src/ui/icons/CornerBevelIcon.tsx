import { Component } from 'solid-js';
import { IconProps } from './types';

const CornerBevelIcon: Component<IconProps> = (props) => (
  <svg
    width="16"
    height="16"
    viewBox="0 0 16 16"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M8 8v6h6V9.914L6.086 2H2v6h6zM6.5 1L15 9.5V15H7V9H1V1h5.5z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CornerBevelIcon;
