#include <stdio.h>
#include <emscripten.h>

#ifdef __INTELLISENSE__
    #define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {
  void EMSCRIPTEN_KEEPALIVE alive() {
    printf("I am being kept alive\n");
  }

  int EMSCRIPTEN_KEEPALIVE add(int a, int b) {
    return a + b;
  }
}
