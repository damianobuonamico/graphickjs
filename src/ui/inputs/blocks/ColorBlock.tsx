import { vec2 } from '@/math';
import { EyeDropperIcon } from '@/ui/icons';
import { Popover } from '@/ui/overlays';
import { HEX2RGB, HSV2RGB, RGB2HEX, RGB2HSV } from '@/utils/color';
import {
  Component,
  createEffect,
  createMemo,
  createSignal,
  Match,
  onCleanup,
  Show,
  Switch
} from 'solid-js';
import Button from '../Button';
import InputWrapper from './InputWrapper';
import Slider from '../Slider';
import Select from '../Select';
import TextBlock from './TextBlock';

const ColorBlock: Component<{
  onInput?(value: vec4): void;
  value: vec4; // [H, S, V, A]
}> = (props) => {
  const onInput = props.onInput ?? (() => {});

  const [eyeDropperActive, setEyeDropperActive] = createSignal(false);
  const [anchor, setAnchor] = createSignal(vec2.create());
  const [format, setFormat] = createSignal('HEX');

  const RGB = createMemo(() => HSV2RGB(props.value.slice(0, 3) as vec3));
  const HEX = createMemo(() => RGB2HEX(RGB()).toUpperCase());

  let pickerRef: HTMLDivElement | undefined;

  createEffect(() => {
    if (pickerRef) {
      const rect = pickerRef.getBoundingClientRect();

      setAnchor(
        vec2.mul(vec2.divS([rect.width || 240, rect.height || 240], 100), [
          props.value[1],
          100 - props.value[2]
        ])
      );
    }
  });

  const setHSV = (hsv: vec3) => {
    onInput([...hsv, props.value[3]]);
  };

  const setAlpha = (a: number) => {
    onInput([props.value[0], props.value[1], props.value[2], a]);
  };

  const onEyeDropper = async () => {
    setEyeDropperActive(true);

    if ('EyeDropper' in window) {
      const eyeDropper = new (window as any).EyeDropper();

      try {
        const result = await eyeDropper.open();
        const rgba = HEX2RGB(result.sRGBHex);
        const hsv = RGB2HSV(rgba.slice(0, 3) as vec3);

        if (rgba.length === 4) {
          onInput([...hsv, rgba[3]]);
        } else {
          setHSV(hsv);
        }

        setEyeDropperActive(false);
      } catch (err) {
        setEyeDropperActive(false);
      }
    }
  };

  const onPointerDown = (e: MouseEvent) => {
    document.addEventListener('pointermove', onPointerMove);
    document.addEventListener('pointerup', onPointerUp);

    if (pickerRef) {
      const rect = pickerRef.getBoundingClientRect();
      const clamped = vec2.clamp(
        vec2.mulS(vec2.div(vec2.sub([e.x, e.y], [rect.x, rect.y]), [rect.width, rect.height]), 100),
        [0, 0],
        [100, 100]
      );

      setHSV([props.value[0], clamped[0], 100 - clamped[1]]);
    }
  };

  const onPointerMove = (e: MouseEvent) => {
    e.preventDefault();

    if (pickerRef) {
      const rect = pickerRef.getBoundingClientRect();
      const clamped = vec2.clamp(
        vec2.mulS(vec2.div(vec2.sub([e.x, e.y], [rect.x, rect.y]), [rect.width, rect.height]), 100),
        [0, 0],
        [100, 100]
      );

      setHSV([props.value[0], clamped[0], 100 - clamped[1]]);
    }
  };

  const onPointerUp = () => {
    document.removeEventListener('pointermove', onPointerMove);
    document.removeEventListener('pointerup', onPointerUp);
  };

  return (
    <Popover translate={[21, -12]}>
      {props.value[3] === 1 ? (
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
              opacity: props.value[3]
            }}
          />
        </div>
      )}

      <div class="w-60 max-w-60">
        <div class="w-full p-3 px-4">
          <a class="font-semibold">Solid</a>
        </div>
        <div
          ref={pickerRef}
          class="w-full aspect-square"
          style={{
            background: `linear-gradient(to top,#000,rgba(255,255,255,0)), linear-gradient(to right,#fff,rgba(0,0,0,0)), hsl(${props.value[0]}, 100%, 50%)`
          }}
          onPointerDown={onPointerDown}
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
            <InputWrapper class="h-fit ml-1" multiline={true}>
              <div class="w-full h-fit grid">
                <Slider
                  style={{
                    'background-image':
                      'linear-gradient(to right, rgb(255, 0, 0), rgb(255, 255, 0), rgb(0, 255, 0), rgb(0, 255, 213), rgb(0, 0, 255), rgb(255, 0, 255), rgb(255, 0, 0))'
                  }}
                  max={360}
                  value={props.value[0]}
                  onInput={(val) => {
                    setHSV([val, props.value[1], props.value[2]]);
                  }}
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
                  value={props.value[3] * 100}
                  onInput={(val) => setAlpha(val / 100)}
                />
              </div>
            </InputWrapper>
          </div>
          <div class="flex mt-1.5">
            <Select
              class="mr-1.5"
              options={[
                { id: 'HEX', label: 'HEX' },
                { id: 'RGB', label: 'RGB' },
                { id: 'HSV', label: 'HSV' }
              ]}
              value={format()}
              onChange={(val: string) => setFormat(val)}
            />
            <Switch
              fallback={
                <InputWrapper>
                  <TextBlock
                    class="w-full"
                    value={HEX()}
                    type="hex"
                    onChange={(value: string) => {
                      const color = HEX2RGB(value);

                      // TODO: fix hex conversions
                      if (color.length === 4) {
                        onInput([...RGB2HSV(color.slice(0, 3) as vec3), color[3]]);
                      } else {
                        setHSV(RGB2HSV(color));
                      }
                    }}
                  />
                  <TextBlock
                    class="w-5"
                    value={Math.round(props.value[3] * 100)}
                    type="int"
                    rightIcon="%"
                    min={0}
                    max={100}
                    onChange={(value: number) => setAlpha(value / 100)}
                  />
                </InputWrapper>
              }
            >
              <Match when={format() === 'RGB'}>
                <InputWrapper>
                  <TextBlock
                    class="w-full"
                    type="int"
                    value={Math.round(RGB()[0] * 255)}
                    min={0}
                    max={255}
                    onChange={(value: number) => setHSV(RGB2HSV([value / 255, RGB()[1], RGB()[2]]))}
                  />
                  <TextBlock
                    class="w-full"
                    type="int"
                    value={Math.round(RGB()[1] * 255)}
                    min={0}
                    max={255}
                    onChange={(value: number) => setHSV(RGB2HSV([RGB()[0], value / 255, RGB()[2]]))}
                  />
                  <TextBlock
                    class="w-full"
                    type="int"
                    value={Math.round(RGB()[2] * 255)}
                    min={0}
                    max={255}
                    onChange={(value: number) => setHSV(RGB2HSV([RGB()[0], RGB()[1], value / 255]))}
                  />
                  <TextBlock
                    class="w-5"
                    value={Math.round(props.value[3] * 100)}
                    type="int"
                    rightIcon="%"
                    min={0}
                    max={100}
                    onChange={(value: number) => setAlpha(value / 100)}
                  />
                </InputWrapper>
              </Match>
              <Match when={format() === 'HSV'}>
                <InputWrapper>
                  <TextBlock
                    class="w-full"
                    type="int"
                    value={Math.round(props.value[0])}
                    min={0}
                    max={360}
                    onChange={(value: number) => setHSV([value, props.value[1], props.value[2]])}
                  />
                  <TextBlock
                    class="w-full"
                    type="int"
                    value={Math.round(props.value[1])}
                    min={0}
                    max={100}
                    onChange={(value: number) => setHSV([props.value[0], value, props.value[2]])}
                  />
                  <TextBlock
                    class="w-full"
                    type="int"
                    value={Math.round(props.value[2])}
                    min={0}
                    max={100}
                    onChange={(value: number) => setHSV([props.value[0], props.value[1], value])}
                  />
                  <TextBlock
                    class="w-5"
                    value={Math.round(props.value[3] * 100)}
                    type="int"
                    rightIcon="%"
                    min={0}
                    max={100}
                    onChange={(value: number) => setAlpha(value / 100)}
                  />
                </InputWrapper>
              </Match>
            </Switch>
          </div>
        </div>
      </div>
    </Popover>
  );
};

export default ColorBlock;
