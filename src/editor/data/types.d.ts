interface GraphickFile {
  name: string;
  workspace: Workspace;
  date: number;
  artboards: ArtboardObject[];
}

interface SaveData {
  state: State;
  viewport: ViewportObject;
  
}
