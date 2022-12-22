import os
from pathlib import Path

EMCC_PATH = '%EMSDK%/upstream/emscripten/emcc'
OPTIMIZE = True

output = '..\src\wasm\editor.js'
files = []
options = ['ALLOW_MEMORY_GROWTH', 'EXPORT_ES6', 'MODULARIZE', 'MIN_WEBGL_VERSION=2', 'MAX_WEBGL_VERSION=2', 'USE_WEBGL2', 'NO_DISABLE_EXCEPTION_CATCHING']

for path in Path('./').rglob('*.cpp'):
  files.append(str(path))

os.system(' '.join([EMCC_PATH, '-Os' if OPTIMIZE else '-O0', *files, '-o ' + output, '-s ' + ' -s '.join(options), '-l embind']))