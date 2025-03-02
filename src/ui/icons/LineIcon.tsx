import { Component } from 'solid-js';
import { IconProps } from './types';

const LineIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M2.05678 12.9419C1.81393 12.6966 1.81495 12.2999 2.05905 12.0558L12.0603 2.05706C12.3044 1.81302 12.6991 1.81404 12.942 2.05935C13.1848 2.30465 13.1838 2.70135 12.9397 2.9454L2.93849 12.9442C2.69438 13.1882 2.29963 13.1872 2.05678 12.9419Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default LineIcon;
