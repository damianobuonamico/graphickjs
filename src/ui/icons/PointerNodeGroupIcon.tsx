import { Component } from 'solid-js';
import { IconProps } from './types';

const PointerNodeGroupIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M6.36584 9.99497C6.74831 9.62408 7.26016 9.41667 7.79293 9.41667H10.9153L4.25 2.95331V12.0467L6.36584 9.99497Z"
      fill={props.color || 'currentColor'}
      fill-opacity="0.35"
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
    <path
      d="M6.36584 9.99498C6.74831 9.62409 7.26016 9.41668 7.79293 9.41668H10.9153L4.25 2.95332V12.0467L6.36584 9.99498ZM3 1.89013V13.1099C3 13.8167 3.8495 14.1763 4.35692 13.6842L7.23602 10.8924C7.38528 10.7476 7.58502 10.6667 7.79293 10.6667H12.0258C12.7445 10.6667 13.0987 9.79266 12.5827 9.29236L4.35692 1.31581C3.8495 0.823768 3 1.18332 3 1.89013Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
    <path
      d="M12 1.5C12 1.22386 11.7761 1 11.5 1C11.2239 1 11 1.22386 11 1.5V3L9.5 3C9.22386 3 9 3.22386 9 3.5C9 3.77614 9.22386 4 9.5 4H11V5.5C11 5.77614 11.2239 6 11.5 6C11.7761 6 12 5.77614 12 5.5V4H13.5C13.7761 4 14 3.77614 14 3.5C14 3.22386 13.7761 3 13.5 3L12 3V1.5Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default PointerNodeGroupIcon;
