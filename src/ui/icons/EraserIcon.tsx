import { Component } from 'solid-js';
import { IconProps } from './types';

const EraserIcon: Component<IconProps> = (props) => (
  <svg
    width="15"
    height="15"
    viewBox="0 0 15 15"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
    {...props}
  >
    <path
      d="M8.38026 0.858582C8.69268 0.546163 9.19921 0.546162 9.51163 0.858582L14.1414 5.48837C14.4538 5.80079 14.4538 6.30732 14.1414 6.61974L8.61974 12.1414C8.32031 12.4409 7.84257 12.4533 7.52832 12.1788L6.70709 13H13.5C13.7761 13 14 13.2239 14 13.5C14 13.7761 13.7761 14 13.5 14H5.70709L5.56569 14.1414C5.25327 14.4538 4.74674 14.4538 4.43432 14.1414L0.858582 10.5657C0.546164 10.2532 0.546162 9.74671 0.858582 9.43429L2.82121 7.47166C2.5467 7.1574 2.55916 6.67969 2.85858 6.38026L8.38026 0.858582ZM3.52702 8.18007L6.81991 11.473L5 13.2929L1.70711 9.99998L3.52702 8.18007Z"
      fill={props.color || 'currentColor'}
      fill-rule="evenodd"
      clip-rule="evenodd"
    />
  </svg>
);

export default EraserIcon;
