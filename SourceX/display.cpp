#include "display.h"
#include "DiabloUI/diabloui.h"
#include "controls/controller.h"
#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"

#ifdef USE_SDL1
#ifndef SDL1_VIDEO_MODE_BPP
#define SDL1_VIDEO_MODE_BPP 0
#endif
#ifndef SDL1_VIDEO_MODE_FLAGS
#define SDL1_VIDEO_MODE_FLAGS SDL_SWSURFACE
#endif
#ifdef SDL1_VIDEO_MODE_WIDTH
#define DEFAULT_WIDTH SDL1_VIDEO_MODE_WIDTH
#endif
#ifdef SDL1_VIDEO_MODE_HEIGHT
#define DEFAULT_HEIGHT SDL1_VIDEO_MODE_HEIGHT
#endif
#endif

#ifndef DEFAULT_WIDTH
#define DEFAULT_WIDTH 640
#endif
#ifndef DEFAULT_HEIGHT
#define DEFAULT_HEIGHT 480
#endif

namespace dvl {

extern SDL_Surface *renderer_texture_surface; /** defined in dx.cpp */

int screenWidth;
int screenHeight;
int viewportHeight;
int borderRight;

#ifdef USE_SDL1
void SetVideoMode(int width, int height, int bpp, uint32_t flags) {
	SDL_Log("Setting video mode %dx%d bpp=%u flags=0x%08X", width, height, bpp, flags);
	SDL_SetVideoMode(width, height, bpp, flags);
	const SDL_VideoInfo &current = *SDL_GetVideoInfo();
	SDL_Log("Video mode is now %dx%d bpp=%u flags=0x%08X",
	    current.current_w, current.current_h, current.vfmt->BitsPerPixel, SDL_GetVideoSurface()->flags);
	ghMainWnd = SDL_GetVideoSurface();
}

void SetVideoModeToPrimary(bool fullscreen, int width, int height) {
	int flags = SDL1_VIDEO_MODE_FLAGS | SDL_HWPALETTE;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	SetVideoMode(width, height, SDL1_VIDEO_MODE_BPP, flags);
	if (OutputRequiresScaling())
		SDL_Log("Using software scaling");
}

bool IsFullScreen() {
	return (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) != 0;
}
#endif

void AdjustToScreenGeometry(int width, int height)
{
	screenWidth = width;
	screenHeight = height;

	borderRight = 64;
	if (screenWidth % 4) {
		// The buffer needs to be devisable by 4 for the engine to blit correctly
		borderRight += 4 - screenWidth % 4;
	}

	viewportHeight = screenHeight;
	if (screenWidth <= PANEL_WIDTH) {
		// Part of the screen is fully obscured by the UI
		viewportHeight -= PANEL_HEIGHT;
	}
}

void CalculatePreferdWindowSize(int &width, int &height, bool useIntegerScaling)
{
#ifdef USE_SDL1
	const SDL_VideoInfo &best = *SDL_GetVideoInfo();
	SDL_Log("Best video mode reported as: %dx%d bpp=%d hw_available=%u",
	    best.current_w, best.current_h, best.vfmt->BitsPerPixel, best.hw_available);
#else
	SDL_DisplayMode mode;
	if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
		ErrSdl();
	}

	if (!useIntegerScaling) {
		float wFactor = (float)mode.w / width;
		float hFactor = (float)mode.h / height;

		if (wFactor > hFactor) {
			width = mode.w * height / mode.h;
		} else {
			height = mode.h * width / mode.w;
		}
		return;
	}

	int wFactor = mode.w / width;
	int hFactor = mode.h / height;

	if (wFactor > hFactor) {
		width = mode.w / hFactor;
		height = mode.h / hFactor;
	} else {
		width = mode.w / wFactor;
		height = mode.h / wFactor;
	}
#endif
}

bool SpawnWindow(const char *lpWindowName)
{
	if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC) <= -1) {
		ErrSdl();
	}

#ifdef USE_SDL1
	SDL_EnableUNICODE(1);
#endif
#ifdef USE_SDL1
	// On SDL 1, there are no ADDED/REMOVED events.
	// Always try to initialize the first joystick.
	Joystick::Add(0);
#ifdef __SWITCH__
	// TODO: There is a bug in SDL2 on Switch where it does not repport controllers on startup (Jan 1, 2020)
	GameController::Add(0);
#endif
#endif

	int width = DEFAULT_WIDTH;
	DvlIntSetting("width", &width);
	int height = DEFAULT_HEIGHT;
	DvlIntSetting("height", &height);
	BOOL integerScalingEnabled = false;
	DvlIntSetting("integer scaling", &integerScalingEnabled);

	if (fullscreen)
		DvlIntSetting("fullscreen", &fullscreen);

	int grabInput = 0;
	DvlIntSetting("grab input", &grabInput);

	BOOL upscale = true;
	DvlIntSetting("upscale", &upscale);
	BOOL oar = false;
	DvlIntSetting("original aspect ratio", &oar);

	if (upscale && !oar) {
		CalculatePreferdWindowSize(width, height, integerScalingEnabled);
	}
	AdjustToScreenGeometry(width, height);

#ifdef USE_SDL1
	if (upscale) {
		upscale = false;
		SDL_Log("upscaling not supported with USE_SDL1");
	}
	SDL_WM_SetCaption(lpWindowName, WINDOW_ICON_NAME);
	SetVideoModeToPrimary(fullscreen, width, height);
	if (grabInput)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	atexit(SDL_VideoQuit); // Without this video mode is not restored after fullscreen.
#else
	int flags = 0;
	if (upscale) {
		if (fullscreen) {
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		flags |= SDL_WINDOW_RESIZABLE;

		char scaleQuality[2] = "2";
		DvlStringSetting("scaling quality", scaleQuality, 2);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality);
	} else if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	if (grabInput) {
		flags |= SDL_WINDOW_INPUT_GRABBED;
	}

	ghMainWnd = SDL_CreateWindow(lpWindowName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
#endif
	if (ghMainWnd == NULL) {
		ErrSdl();
	}

	int refreshRate = 60;
#ifndef USE_SDL1
	SDL_DisplayMode mode;
	SDL_GetDisplayMode(0, 0, &mode);
	if (mode.refresh_rate != 0) {
		refreshRate = mode.refresh_rate;
	}
#endif
	refreshDelay = 1000000 / refreshRate;

	if (upscale) {
#ifndef USE_SDL1
		Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;

		vsyncEnabled = 1;
		DvlIntSetting("vsync", &vsyncEnabled);
		if (vsyncEnabled) {
			rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
		}

		renderer = SDL_CreateRenderer(ghMainWnd, -1, rendererFlags);
		if (renderer == NULL) {
			ErrSdl();
		}

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
		if (texture == NULL) {
			ErrSdl();
		}

		if (integerScalingEnabled && SDL_RenderSetIntegerScale(renderer, SDL_TRUE) < 0) {
			ErrSdl();
		}

		if (SDL_RenderSetLogicalSize(renderer, width, height) <= -1) {
			ErrSdl();
		}
#endif
	} else {
#ifdef USE_SDL1
		const SDL_VideoInfo &current = *SDL_GetVideoInfo();
		width = current.current_w;
		height = current.current_h;
#else
		SDL_GetWindowSize(ghMainWnd, &width, &height);
#endif
		AdjustToScreenGeometry(width, height);
	}

	return ghMainWnd != NULL;
}

SDL_Surface *GetOutputSurface()
{
#ifdef USE_SDL1
	return SDL_GetVideoSurface();
#else
	if (renderer)
		return renderer_texture_surface;
	return SDL_GetWindowSurface(ghMainWnd);
#endif
}

bool OutputRequiresScaling()
{
#ifdef USE_SDL1
	return SCREEN_WIDTH != GetOutputSurface()->w || SCREEN_HEIGHT != GetOutputSurface()->h;
#else // SDL2, scaling handled by renderer.
	return false;
#endif
}

void ScaleOutputRect(SDL_Rect *rect)
{
	if (!OutputRequiresScaling())
		return;
	const SDL_Surface *surface = GetOutputSurface();
	rect->x = rect->x * surface->w / SCREEN_WIDTH;
	rect->y = rect->y * surface->h / SCREEN_HEIGHT;
	rect->w = rect->w * surface->w / SCREEN_WIDTH;
	rect->h = rect->h * surface->h / SCREEN_HEIGHT;
}

#ifdef USE_SDL1
namespace {

SDL_Surface *CreateScaledSurface(SDL_Surface *src)
{
	SDL_Rect stretched_rect = { 0, 0, static_cast<Uint16>(src->w), static_cast<Uint16>(src->h) };
	ScaleOutputRect(&stretched_rect);
	SDL_Surface *stretched = SDL_CreateRGBSurface(
			SDL_SWSURFACE, stretched_rect.w, stretched_rect.h, src->format->BitsPerPixel,
	    src->format->Rmask, src->format->Gmask, src->format->Bmask, src->format->Amask);
	if (SDL_HasColorKey(src)) {
		SDL_SetColorKey(stretched, SDL_SRCCOLORKEY, src->format->colorkey);
		if (src->format->palette != NULL)
			SDL_SetPalette(stretched, SDL_LOGPAL, src->format->palette->colors, 0, src->format->palette->ncolors);
	}
	if (SDL_SoftStretch((src), NULL, stretched, &stretched_rect) < 0) {
		SDL_FreeSurface(stretched);
		ErrSdl();
	}
	return stretched;
}

} // namespace
#endif // USE_SDL1

void ScaleSurfaceToOutput(SDL_Surface **surface)
{
#ifdef USE_SDL1
	if (!OutputRequiresScaling())
		return;
	SDL_Surface *stretched = CreateScaledSurface(*surface);
	SDL_FreeSurface((*surface));
	*surface = stretched;
#endif
}

} // namespace dvl
