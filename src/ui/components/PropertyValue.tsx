import { classNames } from '@/utils/utils';
import { Component, For, JSX, Show } from 'solid-js';

const PropertyValue: Component<{
  children: JSX.Element | JSX.Element[];
  rightPadding?: boolean;
  disablePadding?: boolean;
  fullWidth?: boolean;
  class?: string;
  correctTextPadding?: boolean[] | boolean;
  hoverEffect?: boolean;
}> = (props) => {
  return (
    <div
      class={classNames(
        'border border-transparent rounded property-value flex min-h-[1rem] bg-primary-700',
        props.class,
        {
          'w-full': props.fullWidth,
          'w-fit': !props.fullWidth,
          'hover:border-primary-500': props.hoverEffect !== false
        }
      )}
    >
      {Array.isArray(props.children) ? (
        <For each={props.children}>
          {(item, index) => (
            <>
              <div
                class={classNames('items-center flex', [
                  {
                    'pr-0': !props.rightPadding,
                    'pr-2': props.rightPadding,
                    'pl-2': !(typeof props.correctTextPadding === 'boolean'
                      ? props.correctTextPadding === false
                      : props.correctTextPadding && props.correctTextPadding[index()] === false),
                    'w-full': props.fullWidth && index() == 0
                  },
                  'p-0.5',
                  !props.disablePadding
                ])}
              >
                {item}
              </div>
              <Show when={index() !== (props.children as JSX.Element[]).length - 1}>
                <div class="w-[1px] h-auto border-l border-transparent border-l-primary-800 separator"></div>
              </Show>
            </>
          )}
        </For>
      ) : (
        <div
          class={classNames('items-center flex', [
            {
              'pr-0': props.rightPadding === false,
              'pl-2': props.correctTextPadding === true,
              'w-full': props.fullWidth
            },
            'p-1.5',
            !props.disablePadding
          ])}
        >
          {props.children}
        </div>
      )}
    </div>
  );
};

export default PropertyValue;
