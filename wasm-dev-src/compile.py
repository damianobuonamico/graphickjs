import os
from pathlib import Path

DEBUG = False

EMCC_PATH = '%EMSDK%/upstream/emscripten/emcc'
OUTPUT = '..\public\editor.js'
OPTIONS = [
  'PTHREAD_POOL_SIZE=navigator.hardwareConcurrency',
  'INITIAL_MEMORY=67108864',
  # 'INITIAL_MEMORY=33554432',
  # 'MAXIMUM_MEMORY=536870912',
  'ALLOW_MEMORY_GROWTH', 
  # 'EXPORT_ES6', 
  'MODULARIZE=1', 
  'MIN_WEBGL_VERSION=2',
  'MAX_WEBGL_VERSION=2', 
  'USE_WEBGL2', 
  # 'FULL_ES3',
  'EXPORTED_RUNTIME_METHODS="["cwrap", "ccall", "allocateUTF8"]"',
  'EXPORT_NAME="createModule"'
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
  '-msimd128',
  *files,
  '-o ' + OUTPUT,
  '-lembind',
  '-DEMSCRIPTEN=1',
  '-pthread',
  '-DSIMD_GENERIC',
  '-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0',
  '-s' + ' -s'.join(OPTIONS),
  '-fno-rtti',
  '-fno-exceptions',
  '-std=c++20'
]

if (DEBUG) :
  os.system(' '.join([*COMMON, '-DGK_CONF_DEBUG=1', '-g', '-fdebug-compilation-dir="../wasm-dev-src"']))
else :
  os.system(' '.join([*COMMON, '-DGK_CONF_DIST=1', '-O3']))
