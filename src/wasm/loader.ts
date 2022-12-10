import wasm from './editor';

const fallback: any = () => {};

const API: Api = {
  _init: fallback
};

wasm().then((module: any) => {
  API._init = module._init;

  module._init();
});

export default API;
