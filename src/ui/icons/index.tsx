import { IconProps } from './types';

import AffinityDesignerLogoIcon from './AffinityDesignerLogoIcon';
import AffinityPhotoLogoIcon from './AffinityPhotoLogoIcon';
import AffinityPublisherLogoIcon from './AffinityPublisherLogoIcon';
import CheckIcon from './CheckIcon';
import ChevronRightIcon from './ChevronRightIcon';
import CircleIcon from './CircleIcon';
import EyeDropperIcon from './EyeDropperIcon';
import HandIcon from './HandIcon';
import LoadingIcon from './LoadingIcon';
import PenIcon from './PenIcon';
import PointerIcon from './PointerIcon';
import PointerVertexIcon from './PointerVertexIcon';
import RectangleIcon from './RectangleIcon';
import ZoomIcon from './ZoomIcon';

export { AffinityDesignerLogoIcon };
export { AffinityPhotoLogoIcon };
export { AffinityPublisherLogoIcon };
export { CheckIcon };
export { ChevronRightIcon };
export { CircleIcon };
export { EyeDropperIcon };
export { HandIcon };
export { LoadingIcon };
export { PenIcon };
export { PointerIcon };
export { PointerVertexIcon };
export { RectangleIcon };
export { ZoomIcon };

const getIcon = (name: string, props?: IconProps) => {
  switch (name) {
    case 'affinityDesignerLogo':
      return <AffinityDesignerLogoIcon {...props} />;
    case 'affinityPhotoLogo':
      return <AffinityPhotoLogoIcon {...props} />;
    case 'affinityPublisherLogo':
      return <AffinityPublisherLogoIcon {...props} />;
    case 'check':
      return <CheckIcon {...props} />;
    case 'chevronRight':
      return <ChevronRightIcon {...props} />;
    case 'circle':
      return <CircleIcon {...props} />;
    case 'eyeDropper':
      return <EyeDropperIcon {...props} />;
    case 'hand':
      return <HandIcon {...props} />;
    case 'loading':
      return <LoadingIcon {...props} />;
    case 'pen':
      return <PenIcon {...props} />;
    case 'pointer':
      return <PointerIcon {...props} />;
    case 'pointerVertex':
      return <PointerVertexIcon {...props} />;
    case 'rectangle':
      return <RectangleIcon {...props} />;
    case 'zoom':
      return <ZoomIcon {...props} />;
    default:
      return <></>;
  }
};

export default getIcon;
