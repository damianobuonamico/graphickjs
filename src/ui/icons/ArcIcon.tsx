import { Component } from 'solid-js';
import { IconProps } from './types';

const ArcIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M2.49877 13.1265C2.15444 13.1265 1.87531 12.846 1.87531 12.5C1.87531 7.16295 6.11618 1.87469 12.5 1.87469C12.8443 1.87469 13.1234 2.1552 13.1234 2.50123C13.1234 2.84725 12.8443 3.12776 12.5 3.12776C6.87666 3.12776 3.12223 7.78055 3.12223 12.5C3.12223 12.846 2.8431 13.1265 2.49877 13.1265Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default ArcIcon;
