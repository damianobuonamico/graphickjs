import { vec2 } from '@/math';
import { RGB2HSB, RGB2HEX, HSB2RGB } from '@/utils/color';
import { Component, createEffect, createSignal, createMemo } from 'solid-js';
import { EyeDropperIcon } from '../icons';
import { Button, Slider, Select, Input } from '../inputs';
import { Popover } from '../overlays';
import PropertyValue from './PropertyValue';

const ColorPropertyValue: Component<{
  value: [number, number, number, number];
  // onInput: (color: vec3 | vec4 | string, format?: ColorFormat) => void;
  // onInput: (color: any, format?: any) => void;
  onInput: (color: [number, number, number, number]) => void;
}> = (props) => {
  const [eyeDropperActive, setEyeDropperActive] = createSignal(false);

  const [anchor, setAnchor] = createSignal(vec2.create());
  const [format, setFormat] = createSignal('HEX');

  const [HSB, setHSB] = createSignal(RGB2HSB(props.value.slice(0, 3) as vec3));
  const [A, setA] = createSignal(props.value[3]);

  const RGB = createMemo(() => HSB2RGB(HSB() as vec3));
  const HEX = createMemo(() =>
    RGB2HEX(RGB() as vec3)
      .replace('#', '')
      .toUpperCase()
  );

  // const [color, setColor] = createSignal(props.value.hex);
  // const [value, setValue] = createSignal(props.value.hsb);
  // const [alpha, setAlpha] = createSignal(props.value.alpha);

  createEffect(() => {
    // const hsb = HSB();

    props.onInput([...RGB(), A()]);

    // if (hsb[1] === 0) setValue((prev) => [prev[0], prev[1], hsb[2]]);
    // else if (hsb[2] === 0) setValue((prev) => [prev[0], hsb[1], prev[2]]);
    // else setValue(hsb);
    // setAlpha(props.value.alpha);
    // setColor(props.value.hex);
  });

  let pickerRef: HTMLDivElement | undefined;

  createEffect(() => {
    if (pickerRef) {
      const rect = pickerRef.getBoundingClientRect();

      setAnchor(
        vec2.mul(vec2.divS([rect.width || 240, rect.height || 240], 100), [
          HSB()[1],
          100 - HSB()[2]
        ])
      );
    }
  });

  const onEyeDropper = async () => {
    // setEyeDropperActive(true);
    // if ('EyeDropper' in window) {
    //   const eyeDropper = new (window as any).EyeDropper();
    //   try {
    //     const result = await eyeDropper.open();
    //     const hex = result.sRGBHex;
    //     props.value.set(hex);
    //     props.onInput(result.sRGBHex);
    //     props.onChange();
    //     setColor(hex);
    //     setValue(props.value.hsb);
    //     setEyeDropperActive(false);
    //   } catch (err) {
    //     setEyeDropperActive(false);
    //   }
    // }
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

      setHSB([HSB()[0], clamped[0], 100 - clamped[1]]);
      // props.value.tempSet([value()[0], clamped[0], 100 - clamped[1]], 'hsb');
      // props.onInput(props.value.hex);
      // setColor(props.value.hex);
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

      setHSB([HSB()[0], clamped[0], 100 - clamped[1]]);
      // props.value.tempSet([value()[0], clamped[0], 100 - clamped[1]], 'hsb');
      // props.onInput(props.value.hex);
      // setColor(props.value.hex);
    }
  };

  const onMouseUp = () => {
    document.removeEventListener('mousemove', onMouseMove);
    // props.onChange();
  };

  return (
    <PropertyValue fullWidth={true} rightPadding={true} correctTextPadding={[false, true]}>
      <div class="flex w-full">
        <Popover translate={[19, -13]}>
          {A() === 1 ? (
            <div
              class="w-4 h-4 cursor-pointer rounded-[0.2rem] border-primary-500 border mr-1"
              style={{
                'background-color': '#' + HEX()
              }}
            />
          ) : (
            <div
              class="w-4 h-4 cursor-pointer rounded-[0.2rem] flex border-primary-500 border mr-1"
              style={{
                'background-image':
                  'linear-gradient(45deg, rgb(55, 58, 64) 25%, transparent 25%), linear-gradient(-45deg, rgb(55, 58, 64) 25%, transparent 25%), linear-gradient(45deg, transparent 75%, rgb(55, 58, 64) 75%), linear-gradient(-45deg, rgb(26, 27, 30) 75%, rgb(55, 58, 64) 75%)',
                'background-size': '8px 8px',
                'background-position': '-2px 0px, -2px 4px, 2px -4px, -6px 0px',
                'background-origin': 'right'
              }}
            >
              <div
                class="w-1/2 h-full bg-blue-50 rounded-l-sm"
                style={{
                  'background-color': '#' + HEX()
                }}
              />
              <div
                class="w-1/2 h-full bg-blue-50 rounded-r-sm"
                style={{
                  'background-color': '#' + HEX(),
                  opacity: A()
                }}
              />
            </div>
          )}

          <div class="w-60">
            <div class="w-full p-2 py-3">
              <a class="font-semibold ml-[7px]">Solid</a>
            </div>
            <div
              ref={pickerRef}
              class="w-full aspect-square"
              style={{
                background: `linear-gradient(to top,#000,rgba(255,255,255,0)), linear-gradient(to right,#fff,rgba(0,0,0,0)), hsl(${
                  HSB()[0]
                }, 100%, 50%)`
              }}
              onMouseDown={onMouseDown}
            >
              <div
                class="w-3 h-3 relative -left-1.5 -top-1.5 border-white border-2 rounded-md"
                style={{
                  transform: `translate(${anchor()[0]}px, ${anchor()[1]}px)`,
                  'background-color': '#' + HEX()
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
                      value={HSB()[0]}
                      onInput={(val) => {
                        setHSB([val, HSB()[1], HSB()[2]]);
                        // props.value.tempSet([val, value()[1], value()[2]], 'hsb');
                        // props.onInput(props.value.hex);
                        // setColor(props.value.hex);
                      }}
                      // onChange={props.onChange}
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
                        'background-image': `linear-gradient(90deg, transparent, #${HEX()})`
                      }}
                      value={A() * 100}
                      onInput={(val) => {
                        setA(val / 100);
                        // props.value.tempSet([...value(), val / 100], 'hsb');
                        // props.onInput([...value(), val / 100], 'hsb');
                        // setColor(props.value.hex);
                      }}
                      // onChange={props.onChange}
                    />
                  </div>
                </PropertyValue>
              </div>
              <div class="flex mt-1.5">
                <Select
                  options={[
                    { id: 'HEX', label: 'HEX' },
                    { id: 'RGB', label: 'RGB' },
                    { id: 'HSB', label: 'HSB' }
                  ]}
                  value={format()}
                  onChange={(val) => setFormat(val)}
                />
                <PropertyValue rightPadding={true} fullWidth={true}>
                  <Input class="w-full" value={HEX()} />
                  <Input class="w-6" value={`${Math.round(A() * 100)}`} unit="%" />
                </PropertyValue>
              </div>
            </div>
          </div>
        </Popover>
        <Input class="w-full" value={HEX()} />
      </div>
      <Input class="w-6" value={`${Math.round(A() * 100)}`} unit="%" />
    </PropertyValue>
  );
};

export default ColorPropertyValue;
