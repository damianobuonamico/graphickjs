import { IconProps } from './types';

import AffinityDesignerLogoIcon from './AffinityDesignerLogoIcon';
import AffinityPhotoLogoIcon from './AffinityPhotoLogoIcon';
import AffinityPublisherLogoIcon from './AffinityPublisherLogoIcon';
import CheckIcon from './CheckIcon';
import ChevronRightIcon from './ChevronRightIcon';
import PenIcon from './PenIcon';
import PointerIcon from './PointerIcon';
import PointerVertexIcon from './PointerVertexIcon';

export { AffinityDesignerLogoIcon };
export { AffinityPhotoLogoIcon };
export { AffinityPublisherLogoIcon };
export { CheckIcon };
export { ChevronRightIcon };
export { PenIcon };
export { PointerIcon };
export { PointerVertexIcon };

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
    case 'pen':
      return <PenIcon {...props} />;
    case 'pointer':
      return <PointerIcon {...props} />;
    case 'pointerVertex':
      return <PointerVertexIcon {...props} />;
    default:
      return <></>;
  }
};

export default getIcon;
