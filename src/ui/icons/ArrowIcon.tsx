import { Component } from 'solid-js';
import { IconProps } from './types';

const ArrowIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M2.05678 12.9419C1.81393 12.6966 1.81495 12.2999 2.05905 12.0558L9.61638 4.50035L9.06014 3.9441C8.90663 3.79059 8.84339 3.56804 8.89308 3.35622C8.94277 3.1444 9.09828 2.97367 9.30385 2.90524L12.3039 1.90648C12.5281 1.83186 12.7749 1.89089 12.9416 2.05897C13.1083 2.22705 13.1662 2.47533 13.0911 2.70032L12.0911 5.6991C12.0225 5.90455 11.8532 6.05985 11.6434 6.10972C11.4336 6.15959 11.213 6.09696 11.0601 5.94412L10.5003 5.38424L2.93849 12.9442C2.69438 13.1882 2.29963 13.1872 2.05678 12.9419Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default ArrowIcon;
