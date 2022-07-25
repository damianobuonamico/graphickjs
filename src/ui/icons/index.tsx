import AffinityDesignerLogoIcon from './AffinityDesignerLogoIcon';
import AffinityPhotoLogoIcon from './AffinityPhotoLogoIcon';
import AffinityPublisherLogoIcon from './AffinityPublisherLogoIcon';
import CheckIcon from './CheckIcon';
import ChevronRightIcon from './ChevronRightIcon';
import { IconProps } from './types';

export { AffinityDesignerLogoIcon };
export { AffinityPhotoLogoIcon };
export { AffinityPublisherLogoIcon };
export { CheckIcon };
export { ChevronRightIcon };

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
    default:
      return <></>;
  }
};

export default getIcon;
