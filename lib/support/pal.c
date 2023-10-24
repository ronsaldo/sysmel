#include "pal-common.c"
#include "pal-gdb.c"

#if defined(_WIN32)
#include "pal-windows.c"
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include "pal-unix.c"
#else
#include "pal-stdlib.c"
#endif

#ifdef USE_SDL2
#include "pal-sdl2.c"
#else
#include "pal-null-window.c"
#endif
