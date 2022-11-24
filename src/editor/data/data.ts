import { download, fileDialog } from '@/utils/file';
import { stringify, parse } from 'zipson';
import { isArtboard } from '../ecs/entities/artboard';
import CommandHistory from '../history/history';
import SceneManager from '../scene';
import Viewport from '../viewport';

export function save() {
  const data: GraphickFile = {
    type: 'graphick',
    name: SceneManager.state.name,
    workspace: SceneManager.state.mode,
    viewport: SceneManager.viewport.toJSON(),
    artboards: SceneManager.toJSON()
  };

  download(stringify(data), data.name);
}

export function load() {
  fileDialog({ accept: ['.json'], multiple: false }).then((files) => {
    if (!files.length) return;

    Array.from(files).forEach((file) => {
      const reader = new FileReader();

      reader.onload = () => {
        if (!reader.result || typeof reader.result !== 'string') return;

        const parsed = <GraphickFile>parse(reader.result);
        if (parsed.type !== 'graphick') return;

        if (parsed.viewport) SceneManager.viewport = new Viewport(parsed.viewport);
        SceneManager.clear();

        if (parsed.artboards)
          parsed.artboards.forEach((object) => {
            const entity = SceneManager.fromObject(object);
            if (!entity) return;

            if (isArtboard(entity)) SceneManager.createArtboard(entity);
          });

        if (parsed.workspace) SceneManager.setWorkspace(parsed.workspace);

        CommandHistory.clear();
        SceneManager.setViewportArea();
        SceneManager.render();
      };

      reader.readAsText(file);
    });
  });
}
