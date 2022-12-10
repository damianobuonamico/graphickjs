@echo off
call %EMSDK%\upstream\emscripten\emcc test.cpp -o ..\src\wasm\editor.js -s "EXPORTED_FUNCTIONS=['_alive']" -s ENVIRONMENT='web,worker' -s EXPORT_ES6=1 -s MODULARIZE=1
@REM move ..\src\wasm\editor.wasm ..\public
PAUSE
