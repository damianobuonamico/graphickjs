import { Component, For, JSX, Show } from 'solid-js';
import { classNames } from '@/utils/utils';

const InputWrapper: Component<{
  children: JSX.Element | JSX.Element[];
  multiline?: boolean;
  class?: string;
  reducedLeftPadding?: boolean;
  reducedRightPadding?: boolean;
}> = (props) => {
  return (
    <div
      class={classNames(
        'border border-transparent rounded property-value flex bg-primary-700 w-full hover:border-primary-500',
        {
          'h-input': !props.multiline
        },
        props.class
      )}
    >
      {Array.isArray(props.children) ? (
        <For each={props.children}>
          {(item, index) => (
            <>
              <div
                class={classNames('py-1 px-1.5 items-center flex h-full', {
                  'w-full': index() === 0,
                  'pl-1': props.reducedLeftPadding && index() === 0,
                  'pr-1':
                    props.reducedRightPadding &&
                    index() === (props.children as JSX.Element[]).length - 1
                })}
              >
                {item}
              </div>
              <Show
                when={
                  index() !== (props.children as JSX.Element[]).length - 1 &&
                  (typeof (props.children as any)[index() + 1] != 'function' ||
                    (props.children as any)[index() + 1]() != false)
                }
              >
                <div class="w-[1px] h-auto border-l border-transparent border-l-primary-800 separator"></div>
              </Show>
            </>
          )}
        </For>
      ) : (
        <div
          class={classNames('py-1 px-1.5 items-center flex h-full w-full', {
            'pl-1': props.reducedLeftPadding,
            'pr-1': props.reducedRightPadding
          })}
        >
          {props.children}
        </div>
      )}
    </div>
  );
};

export default InputWrapper;
