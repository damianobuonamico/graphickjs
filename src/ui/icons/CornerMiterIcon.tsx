import { Component } from 'solid-js';
import { IconProps } from './types';

const CornerMiterIcon: Component<IconProps> = (props) => (
  <svg
    width="16"
    height="16"
    viewBox="0 0 16 16"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M8 8v6h6V2H2v6h6zm-1 7V9H1V1h14v14H7z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CornerMiterIcon;
