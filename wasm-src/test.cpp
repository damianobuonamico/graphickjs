#include <stdio.h>
#include <emscripten.h>

extern "C" void EMSCRIPTEN_KEEPALIVE alive() {
  printf("I am being kept alive\n");
}
