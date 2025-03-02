import { Component } from 'solid-js';
import { IconProps } from './types';

const RoundedRectangleIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <rect
      x="2"
      y="2"
      width="11"
      height="11"
      rx="2"
      fill={props.color || 'currentColor'}
      fill-opacity="0.35"
    />
    <path
      d="M4 1C2.34315 1 1 2.34315 1 4V11C1 12.6569 2.34315 14 4 14H11C12.6569 14 14 12.6569 14 11V4C14 2.34315 12.6569 1 11 1H4ZM4 2C2.89543 2 2 2.89543 2 4V11C2 12.1046 2.89543 13 4 13H11C12.1046 13 13 12.1046 13 11V4C13 2.89543 12.1046 2 11 2H4Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default RoundedRectangleIcon;
