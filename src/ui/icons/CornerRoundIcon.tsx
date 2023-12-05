import { Component } from 'solid-js';
import { IconProps } from './types';

const CornerRoundIcon: Component<IconProps> = (props) => (
  <svg
    width="16"
    height="16"
    viewBox="0 0 16 16"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M8 8v6h6v-4c0-4.448-3.552-8-8-8H2v6h6zm-1 7V9H1V1h5c5 0 9 4 9 9v5H7z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default CornerRoundIcon;
