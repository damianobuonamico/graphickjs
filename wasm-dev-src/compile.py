import os
from pathlib import Path

DEBUG = False

EMCC_PATH = '%EMSDK%/upstream/emscripten/emcc'
OUTPUT = '..\src\wasm\editor.js'
OPTIONS = [
  'ALLOW_MEMORY_GROWTH', 
  'EXPORT_ES6', 
  'MODULARIZE', 
  'MIN_WEBGL_VERSION=2',
  'MAX_WEBGL_VERSION=2', 
  'USE_WEBGL2', 
  'FULL_ES3', 
  'NO_DISABLE_EXCEPTION_CATCHING', 
  'EXPORTED_RUNTIME_METHODS="["cwrap", "allocateUTF8"]"'
]

files = []

for path in Path('./').rglob('*.cpp'):
  if str(path)[:5] != 'debug':
    files.append(str(path))

for path in Path('./').rglob('*.c'):
  if str(path)[:5] != 'debug':
    files.append(str(path))

for path in Path('./').rglob('*.cc'):
  if str(path)[:5] != 'debug':
    files.append(str(path))

COMMON = [
  EMCC_PATH,
  *files,
  '-o ' + OUTPUT,
  '-l embind',
  '-DEMSCRIPTEN=1',
  '-s ' + ' -s '.join(OPTIONS)
]

if (DEBUG):
  os.system(' '.join([*COMMON, '-DGK_CONF_DEBUG=1', '-g', '-fdebug-compilation-dir="../wasm-dev-src"']))
else:
  os.system(' '.join([*COMMON, '-DGK_CONF_DIST=1', '-Os']))
