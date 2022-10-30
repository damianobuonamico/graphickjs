import { classNames } from '@/utils/utils';
import { Component, For, JSX, Show } from 'solid-js';

const PropertyValue: Component<{
  children: JSX.Element | JSX.Element[];
  rightPadding?: boolean;
}> = (props) => {
  return (
    <div class="border border-transparent hover:border-primary-500 rounded-sm w-fit select-none property-value flex">
      {Array.isArray(props.children) ? (
        <For each={props.children}>
          {(item, index) => (
            <>
              <div
                class={classNames('p-1.5 items-center flex', {
                  'pr-0': props.rightPadding === false
                })}
              >
                {item}
              </div>
              <Show when={index() !== (props.children as JSX.Element[]).length - 1}>
                <div class="w-[1px] h-auto border-l border-transparent separator"></div>
              </Show>
            </>
          )}
        </For>
      ) : (
        <div
          class={classNames('p-1.5 items-center flex', {
            'pr-0': props.rightPadding === false
          })}
        >
          {props.children}
        </div>
      )}
    </div>
  );
};

export default PropertyValue;
