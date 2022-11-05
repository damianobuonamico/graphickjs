import { Component } from 'solid-js';
import { IconProps } from './types';

const BorderWidthIcon: Component<IconProps> = (props) => (
  <svg
    width="12"
    height="12"
    viewBox="0 0 12 12"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M0 0h12v1H0V0zm0 4h12v2H0V4zm12 5H0v3h12V9z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default BorderWidthIcon;
