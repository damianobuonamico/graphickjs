import { IconProps } from './types';

import AffinityDesignerLogoIcon from './AffinityDesignerLogoIcon';
import AffinityPhotoLogoIcon from './AffinityPhotoLogoIcon';
import AffinityPublisherLogoIcon from './AffinityPublisherLogoIcon';
import CheckIcon from './CheckIcon';
import ChevronRightIcon from './ChevronRightIcon';
import CircleIcon from './CircleIcon';
import PenIcon from './PenIcon';
import PointerIcon from './PointerIcon';
import PointerVertexIcon from './PointerVertexIcon';
import RectangleIcon from './RectangleIcon';

export { AffinityDesignerLogoIcon };
export { AffinityPhotoLogoIcon };
export { AffinityPublisherLogoIcon };
export { CheckIcon };
export { ChevronRightIcon };
export { CircleIcon };
export { PenIcon };
export { PointerIcon };
export { PointerVertexIcon };
export { RectangleIcon };

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
    case 'pen':
      return <PenIcon {...props} />;
    case 'pointer':
      return <PointerIcon {...props} />;
    case 'pointerVertex':
      return <PointerVertexIcon {...props} />;
    case 'rectangle':
      return <RectangleIcon {...props} />;
    default:
      return <></>;
  }
};

export default getIcon;
