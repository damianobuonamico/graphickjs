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
  'EXPORTED_FUNCTIONS="["_malloc", "_free"]"',
  'EXPORTED_RUNTIME_METHODS="["cwrap", "allocateUTF8"]"',
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
  '-std=c++17',
  '-s ' + ' -s '.join(OPTIONS)
]

if (DEBUG):
  os.system(' '.join([*COMMON, '-sASSERTIONS=1', '-sNO_DISABLE_EXCEPTION_CATCHING=1', '-DGK_CONF_DEBUG=1', '-g', '-fdebug-compilation-dir="../wasm-src"']))
else:
  os.system(' '.join([*COMMON, '-DGK_CONF_DIST=1', '-O3', '-fno-rtti', '-fno-exceptions', '-funsafe-math-optimizations', '-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0']))

with open(OUTPUT, 'r') as file:
  filedata = file.read()

filedata = filedata.replace('"stencil":!!HEAP32[a+(8>>2)],"antialias"', '"stencil":!!HEAP32[a+(8>>2)],"desynchronized":true,"antialias"')

with open(OUTPUT, 'w') as file:
  file.write(filedata)
