import { Component } from 'solid-js';
import { Button } from '../inputs';
import AnimationManager from '@/editor/animation';
import { PauseIcon, PlayIcon } from '../icons';

const Timeline: Component<{}> = (props) => {
  return (
    <div class="bg-primary-800 w-full h-full flex items-center border-primary-600 border-t">
      <Button onClick={() => AnimationManager.pause()}>
        <PauseIcon />
      </Button>
      <Button onClick={() => AnimationManager.play()}>
        <PlayIcon />
      </Button>
    </div>
  );
};

export default Timeline;
