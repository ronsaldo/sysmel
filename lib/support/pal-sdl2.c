#include "sysmel/pal-window.h"
#include "SDL.h"

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_initialize(void)
{
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_finalize(void)
{
    SDL_Quit();
}

SYSMEL_PAL_EXTERN_C sysmel_pal_window_t *sysmel_pal_window_create(size_t titleSize, const char *title, int x, int y, int width, int height, uint32_t flags)
{
    (void)flags;
    Uint32 sdlFlags = 0;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_ALLOW_HIGH_DPI)
        sdlFlags |= SDL_WINDOW_ALLOW_HIGHDPI;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_ALWAYS_ON_TOP)
        sdlFlags |= SDL_WINDOW_ALWAYS_ON_TOP;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_BORDERLESS)
        sdlFlags |= SDL_WINDOW_BORDERLESS;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_FULLSCREEN)
        sdlFlags |= SDL_WINDOW_FULLSCREEN;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_FULLSCREEN_DESKTOP)
        sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_HIDDEN)
        sdlFlags |= SDL_WINDOW_HIDDEN;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_POPUP)
        sdlFlags |= SDL_WINDOW_POPUP_MENU;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_RESIZABLE)
        sdlFlags |= SDL_WINDOW_RESIZABLE;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_TOOLTIP)
        sdlFlags |= SDL_WINDOW_TOOLTIP;

    if(flags & SYSMEL_PAL_WINDOW_CREATION_FLAGS_UTILITY)
        sdlFlags |= SDL_WINDOW_UTILITY;

    char *titleCString = malloc(titleSize + 1);
    memcpy(titleCString, title, titleSize);
    titleCString[titleSize] = 0;

    int windowX = x;
    if(windowX < 0)
        windowX = SDL_WINDOWPOS_CENTERED;

    int windowY = y;
    if(windowY < 0)
        windowY = SDL_WINDOWPOS_CENTERED;

    SDL_Window *window = SDL_CreateWindow(titleCString, windowX, windowY, width, height, sdlFlags);

    free(titleCString);
    return (sysmel_pal_window_t*)window;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_show(sysmel_pal_window_t *window)
{
    SDL_ShowWindow((SDL_Window*)window);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_hide(sysmel_pal_window_t *window)
{
    SDL_HideWindow((SDL_Window*)window);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_raise(sysmel_pal_window_t *window)
{
    SDL_RaiseWindow((SDL_Window*)window);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_getSize(sysmel_pal_window_t *window, int *outWidth, int *outHeight)
{
    SDL_GetWindowSize((SDL_Window*)window, outWidth, outHeight);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_destroy(sysmel_pal_window_t *window)
{
    SDL_DestroyWindow((SDL_Window*)window);
}

static void sysmel_pal_sdl2Window_convertMouseButtonEvent(SDL_MouseButtonEvent *sdlEvent, sysmel_pal_mouseButtonEvent_t *palEvent)
{
    palEvent->mouse = sdlEvent->which;
    palEvent->button = sdlEvent->button;
    palEvent->state = sdlEvent->state;
    palEvent->clicks = sdlEvent->clicks;
    palEvent->x = sdlEvent->x;
    palEvent->y = sdlEvent->y;
}

static void sysmel_pal_sdl2Window_convertMouseMotionEvent(SDL_MouseMotionEvent *sdlEvent, sysmel_pal_mouseMotionEvent_t *palEvent)
{
    palEvent->mouse = sdlEvent->which;
    palEvent->state = sdlEvent->state;
    palEvent->x = sdlEvent->x;
    palEvent->y = sdlEvent->y;
    palEvent->deltaX = sdlEvent->xrel;
    palEvent->deltaY = sdlEvent->yrel;
}

static void sysmel_pal_sdl2Window_convertMouseWheelEvent(SDL_MouseWheelEvent *sdlEvent, sysmel_pal_mouseWheelEvent_t *palEvent)
{
    palEvent->mouse = sdlEvent->which;
    palEvent->x = sdlEvent->x;
    palEvent->y = sdlEvent->direction == SDL_MOUSEWHEEL_FLIPPED ? -sdlEvent->y : sdlEvent->y;
}

static void sysmel_pal_sdl2Window_convertKeyboardEvent(SDL_KeyboardEvent *sdlEvent, sysmel_pal_keyboardEvent_t *palEvent)
{
    palEvent->scancode = sdlEvent->keysym.scancode;
    palEvent->symbol = sdlEvent->keysym.sym;
    palEvent->modifiers = sdlEvent->keysym.mod;
}

static void sysmel_pal_sdl2Window_convertTextInputEvent(SDL_TextInputEvent *sdlEvent, sysmel_pal_textInputEvent_t *palEvent)
{
    memcpy(palEvent->text, sdlEvent->text, SYSMEL_PAL_WINDOW_TEXT_INPUT_EVENT_SIZE);
}

static void sysmel_pal_sdl2Window_convertTextEditingEvent(SDL_TextEditingEvent *sdlEvent, sysmel_pal_textEditingEvent_t *palEvent)
{
    memcpy(palEvent->text, sdlEvent->text, SYSMEL_PAL_WINDOW_TEXT_INPUT_EVENT_SIZE);
    palEvent->start = sdlEvent->start;
    palEvent->length = sdlEvent->length;
}

static void sysmel_pal_sdl2Window_convertEvent(SDL_Event *sdlEvent, sysmel_pal_window_event_t *palEvent)
{
    memset(palEvent, 0, sizeof(*palEvent));

    switch(sdlEvent->type)
    {
    case SDL_QUIT:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_QUIT;
        break;

    case SDL_MOUSEBUTTONDOWN:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_BUTTON_DOWN;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertMouseButtonEvent(&sdlEvent->button, &palEvent->data.mouseButton);
        break;
    case SDL_MOUSEBUTTONUP:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_BUTTON_UP;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertMouseButtonEvent(&sdlEvent->button, &palEvent->data.mouseButton);
        break;
    case SDL_MOUSEMOTION:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_MOTION;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertMouseMotionEvent(&sdlEvent->motion, &palEvent->data.mouseMotion);
        break;
    case SDL_MOUSEWHEEL:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MOUSE_WHEEL;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertMouseWheelEvent(&sdlEvent->wheel, &palEvent->data.mouseWheel);
        break;

    case SDL_KEYDOWN:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_KEY_DOWN;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertKeyboardEvent(&sdlEvent->key, &palEvent->data.keyboard);
        break;
    case SDL_KEYUP:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_KEY_UP;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertKeyboardEvent(&sdlEvent->key, &palEvent->data.keyboard);
        break;
    case SDL_TEXTEDITING:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_TEXT_EDITING;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertTextEditingEvent(&sdlEvent->edit, &palEvent->data.textEditing);
        break;
    case SDL_TEXTINPUT:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_TEXT_INPUT;
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->key.windowID);
        sysmel_pal_sdl2Window_convertTextInputEvent(&sdlEvent->text, &palEvent->data.textInput);
        break;
    case SDL_KEYMAPCHANGED:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_KEY_MAP_CHANGED;
        palEvent->window = NULL;
        break;
    case SDL_WINDOWEVENT:
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->window.windowID);

        switch (sdlEvent->window.event)
        {
        case SDL_WINDOWEVENT_SHOWN:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_SHOWN;
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_HIDDEN;
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_EXPOSED;
            break;
        case SDL_WINDOWEVENT_MOVED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MOVED;
            break;
        case SDL_WINDOWEVENT_RESIZED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_RESIZED;
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_SIZE_CHANGED;
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MINIMIZED;
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MAXIMIZED;
            break;
        case SDL_WINDOWEVENT_RESTORED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_RESTORED;
            break;
        case SDL_WINDOWEVENT_ENTER:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_ENTER;
            break;
        case SDL_WINDOWEVENT_LEAVE:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_LEAVE;
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_FOCUS_GAINED;
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_FOCUS_LOST;
            break;
        case SDL_WINDOWEVENT_CLOSE:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_CLOSE;
            break;
        default:
            // By default ignored.
            //printf("Unknown sdl window event 16r%x\n", sdlEvent->window.event);
            break;
        }
        break;
    default:
        //printf("Unknown sdl event 16r%x\n", sdlEvent->type);
        break;
    }
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_pollEvent(sysmel_pal_window_event_t *event)
{
    SDL_Event sdlEvent;
    if(!SDL_PollEvent(&sdlEvent))
        return false;

    sysmel_pal_sdl2Window_convertEvent(&sdlEvent, event);
    return true;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_waitEvent(sysmel_pal_window_event_t *event)
{
    SDL_Event sdlEvent;
    if(!SDL_WaitEvent(&sdlEvent))
        return false;

    sysmel_pal_sdl2Window_convertEvent(&sdlEvent, event);
    return true;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRenderer_t *sysmel_pal_windowRenderer_create(sysmel_pal_window_t *window, uint32_t flags)
{
    (void)flags;
    return (sysmel_pal_windowRenderer_t*)SDL_CreateRenderer((SDL_Window*)window, -1, 0);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_getOutputSize(sysmel_pal_windowRenderer_t *renderer, int *outWidth, int *outHeight)
{
    SDL_GetRendererOutputSize((SDL_Renderer*)renderer, outWidth, outHeight);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_destroy(sysmel_pal_windowRenderer_t *renderer)
{
    SDL_DestroyRenderer((SDL_Renderer*)renderer);
}

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRendererTexture_t *sysmel_pal_windowRenderer_createStreamingTextureBGRA32(sysmel_pal_windowRenderer_t *renderer, int width, int height)
{
    return (sysmel_pal_windowRendererTexture_t*)SDL_CreateTexture((SDL_Renderer*)renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width, height);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_upload(sysmel_pal_windowRendererTexture_t *texture, int x, int y, int width, int height, int pitch, void *pixels)
{
    SDL_Rect lockRegion = {
        .x = x, y = y,
        .w = width, .h = height
    };

    void *destPixels;
    int destPitch;
    if(SDL_LockTexture((SDL_Texture*)texture, &lockRegion, &destPixels, &destPitch))
        return;

    uint8_t *destRow = (uint8_t*)destPixels;
    uint8_t *sourceRow = (uint8_t*)pixels;
    size_t transferRowSize = width * 4;

    for(int cy = 0; cy < height; ++cy)
    {
        memcpy(destRow, sourceRow, transferRowSize);
        destRow += destPitch;
        sourceRow += pitch;
    }

    SDL_UnlockTexture((SDL_Texture*)texture);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRendererTexture_destroy(sysmel_pal_windowRendererTexture_t *texture)
{
    SDL_DestroyTexture((SDL_Texture*)texture);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_beginFrame(sysmel_pal_windowRenderer_t *renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if(!renderer) return;
    SDL_SetRenderDrawColor((SDL_Renderer*)renderer, r, g, b, a);
    SDL_RenderClear((SDL_Renderer*)renderer);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_drawFullTexture(sysmel_pal_windowRenderer_t *renderer, sysmel_pal_windowRendererTexture_t *texture)
{
    SDL_RenderCopy((SDL_Renderer*)renderer, (SDL_Texture*)texture, NULL, NULL);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_endFrame(sysmel_pal_windowRenderer_t *renderer)
{
    if(!renderer) return;
    SDL_RenderPresent((SDL_Renderer*)renderer);
}
