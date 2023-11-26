#ifndef SYSMEL_PAL_WINDOW_H
#define SYSMEL_PAL_WINDOW_H

#include "pal.h"

typedef struct sysmel_pal_window_s sysmel_pal_window_t;
typedef struct sysmel_pal_windowRenderer_s sysmel_pal_windowRenderer_t;
typedef struct sysmel_pal_windowRendererTexture_s sysmel_pal_windowRendererTexture_t;

typedef enum sysmel_pal_windowCreationFlags_e
{
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_NONE = 0,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_ALLOW_HIGH_DPI = 1<<0,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_ALWAYS_ON_TOP = 1<<1,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_BORDERLESS = 1<<2,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_FULLSCREEN = 1<<3,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_FULLSCREEN_DESKTOP = 1<<4,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_HIDDEN = 1<<5,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_POPUP = 1<<6,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_RESIZABLE = 1<<7,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_TOOLTIP = 1<<8,
    SYSMEL_PAL_WINDOW_CREATION_FLAGS_UTILITY = 1<<9,
} sysmel_pal_windowCreationFlags_t;

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

#define SYSMEL_PAL_WINDOW_TEXT_INPUT_EVENT_SIZE 32

typedef struct sysmel_pal_mouseButtonEvent_s
{
    uint32_t mouse;
    uint8_t button;
    uint8_t state;
    uint8_t clicks;
    uint8_t padding;
    int32_t x;
    int32_t y;
} sysmel_pal_mouseButtonEvent_t;

typedef struct sysmel_pal_mouseMotionEvent_s
{
    uint32_t mouse;
    uint32_t state;
    int32_t x;
    int32_t y;
    int32_t deltaX;
    int32_t deltaY;
} sysmel_pal_mouseMotionEvent_t;

typedef struct sysmel_pal_mouseWheelEvent_s
{
    uint32_t mouse;
    int32_t x;
    int32_t y;
} sysmel_pal_mouseWheelEvent_t;

typedef struct sysmel_pal_keyboardEvent_s
{
    uint32_t scancode;
    uint32_t symbol;
    uint32_t modifiers;
} sysmel_pal_keyboardEvent_t;

typedef struct sysmel_pal_textInputEvent_s
{
    char text[SYSMEL_PAL_WINDOW_TEXT_INPUT_EVENT_SIZE];
} sysmel_pal_textInputEvent_t;

typedef struct sysmel_pal_textEditingEvent_s
{
    char text[SYSMEL_PAL_WINDOW_TEXT_INPUT_EVENT_SIZE];
    int32_t start;
    int32_t length;
} sysmel_pal_textEditingEvent_t;

typedef union sysmel_pal_specificEventData_u
{
    sysmel_pal_mouseButtonEvent_t mouseButton;
    sysmel_pal_mouseMotionEvent_t mouseMotion;
    sysmel_pal_mouseWheelEvent_t mouseWheel;
    sysmel_pal_keyboardEvent_t keyboard;
    sysmel_pal_textInputEvent_t textInput;
    sysmel_pal_textEditingEvent_t textEditing;
} sysmel_pal_specificEventData_t;

typedef struct sysmel_pal_window_event_s
{
    uint32_t type;
    sysmel_pal_window_t *window;
    sysmel_pal_specificEventData_t data;
} sysmel_pal_window_event_t;

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_initialize(void);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_finalize(void);

SYSMEL_PAL_EXTERN_C sysmel_pal_window_t *sysmel_pal_window_create(size_t titleSize, const char *title, int x, int y, int width, int height, uint32_t flags);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_show(sysmel_pal_window_t *window);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_hide(sysmel_pal_window_t *window);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_raise(sysmel_pal_window_t *window);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_getSize(sysmel_pal_window_t *window, int *outWidth, int *outHeight);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_destroy(sysmel_pal_window_t *window);

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_pollEvent(sysmel_pal_window_event_t *event);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_waitEvent(sysmel_pal_window_event_t *event);

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRenderer_t *sysmel_pal_windowRenderer_create(sysmel_pal_window_t *window, uint32_t flags);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_getOutputSize(sysmel_pal_windowRenderer_t *renderer, int *outWidth, int *outHeight);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_destroy(sysmel_pal_windowRenderer_t *renderer);

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRendererTexture_t *sysmel_pal_windowRenderer_createStreamingTextureBGRA32(sysmel_pal_windowRenderer_t *renderer, int width, int height);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_upload(sysmel_pal_windowRendererTexture_t *texture, int x, int y, int width, int height, int pitch, void *pixels);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_destroy(sysmel_pal_windowRendererTexture_t *texture);

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_beginFrame(sysmel_pal_windowRenderer_t *renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_drawFullTexture(sysmel_pal_windowRenderer_t *renderer, sysmel_pal_windowRendererTexture_t *texture);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_endFrame(sysmel_pal_windowRenderer_t *renderer);

#endif //SYSMEL_PAL_WINDOW_H
