#ifndef SYSMEL_PAL_WINDOW_H
#define SYSMEL_PAL_WINDOW_H

#include "pal.h"

typedef struct sysmel_pal_window_s sysmel_pal_window_t;
typedef struct sysmel_pal_windowRenderer_s sysmel_pal_windowRenderer_t;
typedef struct sysmel_pal_windowRendererTexture_s sysmel_pal_windowRendererTexture_t;

typedef enum sysmel_pal_window_event_type_e
{
    SYSMEL_PAL_WINDOW_EVENT_TYPE_UNKNOWN = 0,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_QUIT = 1,

    SYSMEL_PAL_WINDOW_EVENT_TYPE_KEY_DOWN = 0x100,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_KEY_UP,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_TEXT_EDITING,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_TEXT_INPUT,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_KEY_MAP_CHANGED,

    SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_BUTTON_DOWN = 0x200,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_BUTTON_UP,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_MOTION,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_WHEEL,

    SYSMEL_PAL_WINDOW_EVENT_TYPE_SHOWN = 0x300,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_HIDDEN,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_EXPOSED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_MOVED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_RESIZED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_SIZE_CHANGED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_MINIMIZED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_MAXIMIZED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_RESTORED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_ENTER,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_LEAVE,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_FOCUS_GAINED,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_FOCUS_LOST,
    SYSMEL_PAL_WINDOW_EVENT_TYPE_CLOSE,
} sysmel_pal_window_event_type_t;

typedef struct sysmel_pal_window_event_s
{
    uint32_t type;
    sysmel_pal_window_t *window;
} sysmel_pal_window_event_t;

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_initialize(void);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_finalize(void);

SYSMEL_PAL_EXTERN_C sysmel_pal_window_t *sysmel_pal_window_create(size_t titleSize, const char *title, int x, int y, int width, int height, uint32_t flags);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_show(sysmel_pal_window_t *window);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_hide(sysmel_pal_window_t *window);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_raise(sysmel_pal_window_t *window);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_destroy(sysmel_pal_window_t *window);

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_pollEvent(sysmel_pal_window_event_t *event);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_waitEvent(sysmel_pal_window_event_t *event);

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRenderer_t *sysmel_pal_windowRenderer_create(sysmel_pal_window_t *window, uint32_t flags);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_destroy(sysmel_pal_windowRenderer_t *renderer);

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRendererTexture_t *sysmel_pal_windowRenderer_createStreamingTextureBGRA32(sysmel_pal_windowRenderer_t *renderer, int width, int height);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_upload(sysmel_pal_windowRendererTexture_t *texture, int x, int y, int width, int height, int pitch, void *pixels);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_destroy(sysmel_pal_windowRendererTexture_t *texture);

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_beginFrame(sysmel_pal_windowRenderer_t *renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_drawFullTexture(sysmel_pal_windowRenderer_t *renderer, sysmel_pal_windowRendererTexture_t *texture);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_endFrame(sysmel_pal_windowRenderer_t *renderer);

#endif //SYSMEL_PAL_WINDOW_H
