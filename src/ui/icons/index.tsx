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
import PlusIcon from './PlusIcon';
import MinusIcon from './MinusIcon';
import EyeClosedIcon from './EyeClosedIcon';
import EyeOpenIcon from './EyeOpenIcon';
import BorderWidthIcon from './BorderWidthIcon';
import CornerBevelIcon from './CornerBevelIcon';
import CornerMiterIcon from './CornerMiterIcon';
import CornerRoundIcon from './CornerRoundIcon';

export { AffinityDesignerLogoIcon };
export { AffinityPhotoLogoIcon };
export { AffinityPublisherLogoIcon };
export { BorderWidthIcon };
export { CheckIcon };
export { ChevronRightIcon };
export { CircleIcon };
export { CornerBevelIcon };
export { CornerMiterIcon };
export { CornerRoundIcon };
export { EyeClosedIcon };
export { EyeDropperIcon };
export { EyeOpenIcon };
export { HandIcon };
export { LoadingIcon };
export { MinusIcon };
export { PenIcon };
export { PlusIcon };
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
    case 'borderWidth':
      return <BorderWidthIcon {...props} />;
    case 'check':
      return <CheckIcon {...props} />;
    case 'chevronRight':
      return <ChevronRightIcon {...props} />;
    case 'circle':
      return <CircleIcon {...props} />;
    case 'cornerBevel':
      return <CornerBevelIcon {...props} />;
    case 'cornerMiter':
      return <CornerMiterIcon {...props} />;
    case 'cornerRound':
      return <CornerRoundIcon {...props} />;
    case 'eyeClosed':
      return <EyeClosedIcon {...props} />;
    case 'eyeDropper':
      return <EyeDropperIcon {...props} />;
    case 'eyeOpen':
      return <EyeOpenIcon {...props} />;
    case 'hand':
      return <HandIcon {...props} />;
    case 'loading':
      return <LoadingIcon {...props} />;
    case 'minus':
      return <MinusIcon {...props} />;
    case 'pen':
      return <PenIcon {...props} />;
    case 'plus':
      return <PlusIcon {...props} />;
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
