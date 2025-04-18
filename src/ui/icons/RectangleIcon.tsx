import { Component } from 'solid-js';
import { IconProps } from './types';

const RectangleIcon: Component<IconProps> = (props) => (
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
      fill={props.color || 'currentColor'}
      fill-opacity="0.35"
    />
    <path
      d="M2 1C1.44772 1 1 1.44772 1 2V13C1 13.5523 1.44772 14 2 14H13C13.5523 14 14 13.5523 14 13V2C14 1.44772 13.5523 1 13 1H2ZM13 2H2V13H13V2Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default RectangleIcon;
