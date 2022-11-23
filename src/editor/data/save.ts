import { LOCAL_STORAGE_KEY_STATE } from '@/utils/constants';

self.onmessage = (message) => {
  const data = <SaveData>message.data;
  const file: GraphickFile = {
    name: data.state.name,
    workspace: data.state.mode,
    date: Date.now(),
    artboards: []
  };
  console.log(message.data);
};

export default null;
