#pragma once

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef __INTELLISENSE__
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <stdint.h>
#include <string>

#include "utils/uuid.h"
#include "utils/console.h"
#include "utils/defines.h"
