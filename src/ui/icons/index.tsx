import { IconProps } from './types';

import AffinityDesignerLogoIcon from './AffinityDesignerLogoIcon';
import AffinityPhotoLogoIcon from './AffinityPhotoLogoIcon';
import AffinityPublisherLogoIcon from './AffinityPublisherLogoIcon';
import BorderDashedIcon from './BorderDashedIcon';
import BorderDottedIcon from './BorderDottedIcon';
import BorderSolidIcon from './BorderSolidIcon';
import BorderWidthIcon from './BorderWidthIcon';
import CapButtIcon from './CapButtIcon';
import CapRoundIcon from './CapRoundIcon';
import CapSquareIcon from './CapSquareIcon';
import CaretDownIcon from './CaretDownIcon';
import CaretLeftIcon from './CaretLeftIcon';
import CaretRightIcon from './CaretRightIcon';
import CaretUpIcon from './CaretUpIcon';
import CheckIcon from './CheckIcon';
import ChevronRightIcon from './ChevronRightIcon';
import CircleIcon from './CircleIcon';
import EraserIcon from './EraserIcon';
import EyeClosedIcon from './EyeClosedIcon';
import EyeDropperIcon from './EyeDropperIcon';
import EyeOpenIcon from './EyeOpenIcon';
import HandIcon from './HandIcon';
import JoinBevelIcon from './JoinBevelIcon';
import JoinMiterIcon from './JoinMiterIcon';
import JoinRoundIcon from './JoinRoundIcon';
import LoadingIcon from './LoadingIcon';
import MinusIcon from './MinusIcon';
import PauseIcon from './PauseIcon';
import PencilIcon from './PencilIcon';
import PenIcon from './PenIcon';
import PlayIcon from './PlayIcon';
import PlusIcon from './PlusIcon';
import PointerIcon from './PointerIcon';
import PointerVertexIcon from './PointerVertexIcon';
import RectangleIcon from './RectangleIcon';
import StopIcon from './StopIcon';
import ZoomIcon from './ZoomIcon';

export { AffinityDesignerLogoIcon };
export { AffinityPhotoLogoIcon };
export { AffinityPublisherLogoIcon };
export { BorderDashedIcon };
export { BorderDottedIcon };
export { BorderSolidIcon };
export { BorderWidthIcon };
export { CapButtIcon };
export { CapRoundIcon };
export { CapSquareIcon };
export { CaretDownIcon };
export { CaretLeftIcon };
export { CaretRightIcon };
export { CaretUpIcon };
export { CheckIcon };
export { ChevronRightIcon };
export { CircleIcon };
export { EraserIcon };
export { EyeClosedIcon };
export { EyeDropperIcon };
export { EyeOpenIcon };
export { HandIcon };
export { JoinBevelIcon };
export { JoinMiterIcon };
export { JoinRoundIcon };
export { LoadingIcon };
export { MinusIcon };
export { PauseIcon };
export { PencilIcon };
export { PenIcon };
export { PlayIcon };
export { PlusIcon };
export { PointerIcon };
export { PointerVertexIcon };
export { RectangleIcon };
export { StopIcon };
export { ZoomIcon };

const getIcon = (name: string, props?: IconProps) => {
  switch (name) {
    case 'affinityDesignerLogo':
      return <AffinityDesignerLogoIcon {...props} />;
    case 'affinityPhotoLogo':
      return <AffinityPhotoLogoIcon {...props} />;
    case 'affinityPublisherLogo':
      return <AffinityPublisherLogoIcon {...props} />;
    case 'borderDashed':
      return <BorderDashedIcon {...props} />;
    case 'borderDotted':
      return <BorderDottedIcon {...props} />;
    case 'borderSolid':
      return <BorderSolidIcon {...props} />;
    case 'borderWidth':
      return <BorderWidthIcon {...props} />;
    case 'capButt':
      return <CapButtIcon {...props} />;
    case 'capRound':
      return <CapRoundIcon {...props} />;
    case 'capSquare':
      return <CapSquareIcon {...props} />;
    case 'caretDown':
      return <CaretDownIcon {...props} />;
    case 'caretLeft':
      return <CaretLeftIcon {...props} />;
    case 'caretRight':
      return <CaretRightIcon {...props} />;
    case 'caretUp':
      return <CaretUpIcon {...props} />;
    case 'check':
      return <CheckIcon {...props} />;
    case 'chevronRight':
      return <ChevronRightIcon {...props} />;
    case 'circle':
      return <CircleIcon {...props} />;
    case 'eraser':
      return <EraserIcon {...props} />;
    case 'eyeClosed':
      return <EyeClosedIcon {...props} />;
    case 'eyeDropper':
      return <EyeDropperIcon {...props} />;
    case 'eyeOpen':
      return <EyeOpenIcon {...props} />;
    case 'hand':
      return <HandIcon {...props} />;
    case 'joinBevel':
      return <JoinBevelIcon {...props} />;
    case 'joinMiter':
      return <JoinMiterIcon {...props} />;
    case 'joinRound':
      return <JoinRoundIcon {...props} />;
    case 'loading':
      return <LoadingIcon {...props} />;
    case 'minus':
      return <MinusIcon {...props} />;
    case 'pause':
      return <PauseIcon {...props} />;
    case 'pencil':
      return <PencilIcon {...props} />;
    case 'pen':
      return <PenIcon {...props} />;
    case 'play':
      return <PlayIcon {...props} />;
    case 'plus':
      return <PlusIcon {...props} />;
    case 'pointer':
      return <PointerIcon {...props} />;
    case 'pointerVertex':
      return <PointerVertexIcon {...props} />;
    case 'rectangle':
      return <RectangleIcon {...props} />;
    case 'stop':
      return <StopIcon {...props} />;
    case 'zoom':
      return <ZoomIcon {...props} />;
    default:
      return undefined;
  }
};

export default getIcon;
