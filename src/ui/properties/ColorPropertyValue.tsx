import { vec2, vec3 } from '@/math';
import { Component, createEffect, createSignal } from 'solid-js';
import { EyeDropperIcon } from '../icons';
import { Button, Slider } from '../inputs';
import { Popover } from '../overlays';
import PropertyValue from './PropertyValue';

const ColorPropertyValue: Component<{
  value: ColorComponent;
  onInput: (color: string) => void;
  onChange: () => void;
}> = (props) => {
  const [eyeDropperActive, setEyeDropperActive] = createSignal(false);
  const [color, setColor] = createSignal(props.value.hex);
  const [value, setValue] = createSignal(vec3.create());
  const [anchor, setAnchor] = createSignal(vec2.create());
  const [alpha, setAlpha] = createSignal(props.value.alpha);

  let pickerRef: HTMLDivElement | undefined;

  createEffect(() => {
    const hsb = props.value.hsb;
    setValue(hsb);
  });

  createEffect(() => {
    if (pickerRef) {
      const rect = pickerRef.getBoundingClientRect();

      setAnchor(
        vec2.mul(vec2.divS([rect.width || 240, rect.height || 240], 100), [
          value()[1],
          100 - value()[2]
        ])
      );
    }
  });

  const onEyeDropper = async () => {
    setEyeDropperActive(true);

    if ('EyeDropper' in window) {
      const eyeDropper = new (window as any).EyeDropper();

      try {
        const result = await eyeDropper.open();
        const hex = result.sRGBHex;
        props.value.set(hex);
        props.onInput(result.sRGBHex);
        props.onChange();
        setColor(hex);
        setValue(props.value.hsb);

        setEyeDropperActive(false);
      } catch (err) {
        setEyeDropperActive(false);
      }
    }
  };

  const onMouseDown = (e: MouseEvent) => {
    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);

    if (pickerRef) {
      const rect = pickerRef.getBoundingClientRect();
      const clamped = vec2.clamp(
        vec2.mulS(vec2.div(vec2.sub([e.x, e.y], [rect.x, rect.y]), [rect.width, rect.height]), 100),
        [0, 0],
        [100, 100]
      );

      setValue([value()[0], clamped[0], 100 - clamped[1]]);
      props.value.tempSet([value()[0], clamped[0], 100 - clamped[1]], 'hsb');
      props.onInput(props.value.hex);
      setColor(props.value.hex);
    }
  };

  const onMouseMove = (e: MouseEvent) => {
    e.preventDefault();

    if (pickerRef) {
      const rect = pickerRef.getBoundingClientRect();
      const clamped = vec2.clamp(
        vec2.mulS(vec2.div(vec2.sub([e.x, e.y], [rect.x, rect.y]), [rect.width, rect.height]), 100),
        [0, 0],
        [100, 100]
      );

      setValue([value()[0], clamped[0], 100 - clamped[1]]);
      props.value.tempSet([value()[0], clamped[0], 100 - clamped[1]], 'hsb');
      props.onInput(props.value.hex);
      setColor(props.value.hex);
    }
  };

  const onMouseUp = () => {
    document.removeEventListener('mousemove', onMouseMove);
    props.onChange();
  };

  return (
    <PropertyValue rightPadding={false} correctTextPadding={[false, true]}>
      <div class="flex">
        <Popover translate={[15, -7]}>
          <div
            class="w-[18px] h-[18px] cursor-pointer rounded-sm"
            style={{
              'background-color': color()
            }}
          />
          <div class="w-60">
            <div class="w-full p-2 py-3">
              <a class="font-semibold select-none ml-[7px]">Solid</a>
            </div>
            <div
              ref={pickerRef}
              class="w-full aspect-square"
              style={{
                background: `linear-gradient(to top,#000,rgba(255,255,255,0)), linear-gradient(to right,#fff,rgba(0,0,0,0)), hsl(${
                  value()[0]
                }, 100%, 50%)`
              }}
              onMouseDown={onMouseDown}
            >
              <div
                class="w-3 h-3 relative -left-1.5 -top-1.5 border-white border-2 rounded-md"
                style={{
                  transform: `translate(${anchor()[0]}px, ${anchor()[1]}px)`,
                  'background-color': color()
                }}
              />
            </div>
            <div class="w-full p-2">
              <div class="w-full flex items-center">
                <Button
                  onClick={onEyeDropper}
                  variant="button"
                  active={eyeDropperActive()}
                  style={{ 'margin-left': '0.1rem' }}
                >
                  <EyeDropperIcon />
                </Button>
                <PropertyValue fullWidth={true} class="h-fit ml-1">
                  <div class="w-full h-fit grid">
                    <Slider
                      style={{
                        'background-image':
                          'linear-gradient(to right, rgb(255, 0, 0), rgb(255, 255, 0), rgb(0, 255, 0), rgb(0, 255, 213), rgb(0, 0, 255), rgb(255, 0, 255), rgb(255, 0, 0))'
                      }}
                      max={360}
                      value={value()[0]}
                      onInput={(val) => {
                        setValue([val, value()[1], value()[2]]);
                        props.value.tempSet([val, value()[1], value()[2]], 'hsb');
                        props.onInput(props.value.hex);
                        setColor(props.value.hex);
                      }}
                      onChange={props.onChange}
                    />
                    <Slider
                      class="mt-2"
                      style={{
                        'background-image':
                          'linear-gradient(45deg, rgb(55, 58, 64) 25%, transparent 25%), linear-gradient(-45deg, rgb(55, 58, 64) 25%, transparent 25%), linear-gradient(45deg, transparent 75%, rgb(55, 58, 64) 75%), linear-gradient(-45deg, rgb(26, 27, 30) 75%, rgb(55, 58, 64) 75%)',
                        'background-size': '8px 8px',
                        'background-position': '0px 0px, 0px 4px, 4px -4px, -4px 0px'
                      }}
                      overlay={{
                        'background-image': `linear-gradient(90deg, transparent, ${color()})`
                      }}
                      value={alpha() * 100}
                      onInput={(val) => {
                        setAlpha(val / 100);
                        props.value.alpha = val / 100;
                      }}
                    />
                  </div>
                </PropertyValue>
              </div>
              <div class="flex mt-1.5">
                <PropertyValue rightPadding={false} fullWidth={true}>
                  <a class="pr-2">Hex</a>
                  <input
                    class="w-full bg-transparent outline-none"
                    type="text"
                    value={color().replace('#', '').toUpperCase()}
                  />
                  <input
                    class="w-10 bg-transparent outline-none"
                    type="text"
                    value={`${Math.round(alpha() * 100)}%`}
                  />
                </PropertyValue>
              </div>
            </div>
          </div>
        </Popover>
        <input
          class="ml-1.5 w-16 bg-transparent outline-none"
          type="text"
          value={color().replace('#', '').toUpperCase()}
        />
      </div>
      <input
        class="w-10 bg-transparent outline-none"
        type="text"
        value={`${Math.round(alpha() * 100)}%`}
      />
    </PropertyValue>
  );
};

export default ColorPropertyValue;
