interface GraphickFile {
  type: 'graphick';
  name: string;
  workspace: Workspace;
  viewport: ViewportObject;
  artboards: EntityObject[];
}
