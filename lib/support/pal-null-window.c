#include "sysmel/pal-window.h"

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_initialize(void)
{
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_finalize(void)
{
}

SYSMEL_PAL_EXTERN_C sysmel_pal_window_t *sysmel_pal_window_create(size_t titleSize, const char *title, int x, int y, int width, int height, uint32_t flags)
{
    (void)titleSize;
    (void)title;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)flags;
    return NULL;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_show(sysmel_pal_window_t *window)
{
    (void)window;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_hide(sysmel_pal_window_t *window)
{
    (void)window;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_raise(sysmel_pal_window_t *window)
{
    (void)window;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_destroy(sysmel_pal_window_t *window)
{
    (void)window;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_pollEvent(sysmel_pal_window_event_t *event)
{
    (void)event;
    return false;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_waitEvent(sysmel_pal_window_event_t *event)
{
    (void)event;
    return false;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRenderer_t *sysmel_pal_windowRenderer_create(sysmel_pal_window_t *window, uint32_t flags)
{
    (void)window;
    (void)flags;
    return NULL;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRendererTexture_t *sysmel_pal_windowRenderer_createStreamingTextureBGRA32(sysmel_pal_windowRenderer_t *renderer, int width, int height)
{
    (void)renderer;
    (void)width;
    (void)height;
    return NULL;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_upload(sysmel_pal_windowRendererTexture_t *texture, int x, int y, int width, int height, int pitch, void *pixels)
{
    (void)texture;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)pitch;
    (void)pixels;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_destroy(sysmel_pal_windowRendererTexture_t *texture)
{
    (void)texture;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_destroy(sysmel_pal_windowRenderer_t *renderer)
{
    (void)renderer;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_beginFrame(sysmel_pal_windowRenderer_t *renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    (void)renderer;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_drawFullTexture(sysmel_pal_windowRenderer_t *renderer, sysmel_pal_windowRendererTexture_t *texture)
{
    (void)renderer;
    (void)texture;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_endFrame(sysmel_pal_windowRenderer_t *renderer)
{
    (void)renderer;
}
