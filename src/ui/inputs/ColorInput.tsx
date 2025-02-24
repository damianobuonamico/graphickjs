import { Component, createEffect, createMemo, createSignal, Show } from 'solid-js';
import InputWrapper from './blocks/InputWrapper';
import TextBlock from './blocks/TextBlock';
import ColorBlock from './blocks/ColorBlock';
import { HEX2RGB, HSV2RGB, RGB2HEX, RGB2HSV } from '@/utils/color';

const ColorInput: Component<{ onInput?(value: vec4): void; value: vec4 | 'mixed' }> = (props) => {
  const onInput = props.onInput ?? (() => {});

  const [mixed, setMixed] = createSignal(props.value === 'mixed');
  const [HSVA, setHSVA] = createSignal([
    ...RGB2HSV(props.value.slice(0, 3) as vec3),
    props.value[3]
  ] as vec4);

  const HEX = createMemo(() => RGB2HEX(HSV2RGB(HSVA().slice(0, 3) as vec3)).toUpperCase());

  createEffect(() => {
    if (props.value === 'mixed') {
      setMixed(true);
      setHSVA([0, 0, 0, 1]);
    } else {
      setMixed(false);
      setHSVA([...RGB2HSV(props.value.slice(0, 3) as vec3), props.value[3]]);
    }
  });

  const setHSVA2RGBA = (hsva: vec4) => {
    setMixed(false);
    setHSVA(hsva);
    onInput([...HSV2RGB(hsva.slice(0, 3) as vec3), hsva[3]] as vec4);
  };

  return (
    <InputWrapper reducedLeftPadding>
      <div class="flex">
        <ColorBlock value={HSVA()} onInput={(color) => setHSVA2RGBA(color)} />
        <TextBlock
          value={mixed() ? '' : HEX()}
          placeholder={mixed() ? 'mixed' : 'HEX'}
          type="hex"
          onChange={(value: string) => {
            const color = HEX2RGB(value);

            // TODO: fix hex conversions
            if (color.length === 4) {
              setHSVA2RGBA([...RGB2HSV(color.slice(0, 3) as vec3), color[3]]);
            } else {
              setHSVA2RGBA([...RGB2HSV(color.slice(0, 3) as vec3), HSVA()[3]]);
            }
          }}
        />
      </div>
      {!mixed() && (
        <TextBlock
          value={Math.round(HSVA()[3] * 100)}
          type="int"
          rightIcon="%"
          class="w-5"
          min={0}
          max={100}
          onChange={(value: number) => setHSVA2RGBA([...(HSVA().slice(0, 3) as vec3), value / 100])}
        />
      )}
    </InputWrapper>
  );
};

export default ColorInput;
