/**
 * @file dx.cpp
 *
 * Implementation of functions setting up the graphics pipeline.
 */
#include "all.h"
#include "../3rdParty/Storm/Source/storm.h"
#include "display.h"
#include <SDL.h>

namespace dvl {

int sgdwLockCount;
BYTE *gpBuffer;
BYTE* gpBuffer_important = 0; //Fluffy: Used for wall transparency
#ifdef _DEBUG
int locktbl[256];
#endif
static CCritSect sgMemCrit;

int vsyncEnabled;
int refreshDelay;
SDL_Renderer *renderer;
SDL_Texture *texture;

/** Currently active palette */
SDL_Palette *palette;
unsigned int pal_surface_palette_version = 0;

/** 24-bit renderer texture surface */
SDL_Surface *renderer_texture_surface = NULL;

/** 8-bit surface wrapper around #gpBuffer */
SDL_Surface *pal_surface;

SDL_Surface *surface_walls = NULL; //Fluffy: For adjust lightmap based on wall pixels
SDL_Texture *texture_walls = NULL; //Fluffy: Used as intermediate texture for surface_walls
BYTE *lightmap_walls = NULL;
SDL_Texture *texture_intermediate = 0; //Fluffy: Game renders to this texture, and then this texture gets presented to the final screen
SDL_Texture *texture_lightmap = 0; //Fluffy: Only gets created if both options_hwRendering and options_lightmapping are true
bool dx_useLightmap = false; //Fluffy: True if in dungeon, but false otherwise
BYTE dx_fade = 0.0f; //Fluffy: If above 0, we apply fading by rendering a black rectangle
SDL_Palette *surface_walls_palette = NULL; //Fluffy debug
BYTE *lightmap_wall_buffer = NULL;

static void dx_create_back_buffer()
{
	pal_surface = SDL_CreateRGBSurfaceWithFormat(0, BUFFER_WIDTH, BUFFER_HEIGHT, 8, SDL_PIXELFORMAT_INDEX8);
	if (pal_surface == NULL) {
		ErrSdl();
	}

	gpBuffer = (BYTE *)pal_surface->pixels;
	if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) {
		gpBuffer_important = new BYTE[BUFFER_WIDTH * BUFFER_HEIGHT]; //Fluffy: Create buffer used for wall transparency
	}
	if (options_hwRendering) {
		texture_intermediate = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT); //Fluffy: Create the texture we use for rendering game graphics
		SDL_SetTextureBlendMode(texture_intermediate, SDL_BLENDMODE_BLEND);

		if (options_lightmapping) {
			texture_lightmap = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
			SDL_SetTextureBlendMode(texture_lightmap, SDL_BLENDMODE_MOD);
			//SDL_SetTextureBlendMode(texture_lightmap, SDL_BLENDMODE_NONE); //Uncomment this to render only lightmap to screen

			//Fluffy: For walls affecting lightmap
			surface_walls_palette = SDL_AllocPalette(256);
			SDL_Color color[256];
			for (int i = 0; i < 256; i++) {
				color[i].r = i;
				color[i].g = i;
				color[i].b = i;
				color[i].a = 255;
			}
			if(SDL_SetPaletteColors(surface_walls_palette, color, 0, 256) < 0)
				ErrSdl();
			surface_walls = SDL_CreateRGBSurfaceWithFormat(0, BUFFER_WIDTH, BUFFER_HEIGHT, 8, SDL_PIXELFORMAT_INDEX8);
			if (surface_walls == NULL)
				ErrSdl();
			if(SDL_SetSurfacePalette(surface_walls, surface_walls_palette) < 0)
				ErrSdl();
			lightmap_walls = (BYTE *)surface_walls->pixels;
			memset(lightmap_walls, 255, BUFFER_WIDTH * BUFFER_HEIGHT);
			texture_walls = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
			if (texture_walls == NULL)
				ErrSdl();
			if(SDL_SetTextureBlendMode(texture_walls, SDL_BLENDMODE_MOD) < 0)
				ErrSdl();
			lightmap_wall_buffer = new BYTE[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
		}
	}

#ifndef USE_SDL1
	// In SDL2, `pal_surface` points to the global `palette`.
	if (SDL_SetSurfacePalette(pal_surface, palette) < 0)
		ErrSdl();
#else
	// In SDL1, `pal_surface` owns its palette and we must update it every
	// time the global `palette` is changed. No need to do anything here as
	// the global `palette` doesn't have any colors set yet.
#endif

	pal_surface_palette_version = 1;
}

static void dx_create_primary_surface()
{
#ifndef USE_SDL1
	if (renderer) {
		int width, height;
		SDL_RenderGetLogicalSize(renderer, &width, &height);
		Uint32 format;
		if (SDL_QueryTexture(texture, &format, NULL, NULL, NULL) < 0)
			ErrSdl();
		renderer_texture_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, SDL_BITSPERPIXEL(format), format);
	}
#endif
	if (GetOutputSurface() == NULL) {
		ErrSdl();
	}
}

void dx_init(HWND hWnd)
{
	SDL_RaiseWindow(ghMainWnd);
	SDL_ShowWindow(ghMainWnd);

	dx_create_primary_surface();
	palette_init();
	dx_create_back_buffer();
}
static void lock_buf_priv()
{
	sgMemCrit.Enter();
	if (sgdwLockCount != 0) {
		sgdwLockCount++;
		return;
	}

	gpBuffer = (BYTE *)pal_surface->pixels;
	gpBufEnd += (uintptr_t)(BYTE *)pal_surface->pixels;
	if (options_hwRendering && options_lightmapping)
		lightmap_walls = (BYTE *)surface_walls->pixels; //Fluffy debug
	sgdwLockCount++;
}

void lock_buf(BYTE idx)
{
#ifdef _DEBUG
	++locktbl[idx];
#endif
	lock_buf_priv();
}

static void unlock_buf_priv()
{
	if (sgdwLockCount == 0)
		app_fatal("draw main unlock error");
	if (gpBuffer == NULL)
		app_fatal("draw consistency error");

	sgdwLockCount--;
	if (sgdwLockCount == 0) {
		gpBufEnd -= (uintptr_t)gpBuffer;
	}
	sgMemCrit.Leave();
}

void unlock_buf(BYTE idx)
{
#ifdef _DEBUG
	if (!locktbl[idx])
		app_fatal("Draw lock underflow: 0x%x", idx);
	--locktbl[idx];
#endif
	unlock_buf_priv();
}

void dx_cleanup()
{
	if (gpBuffer_important)
		delete[] gpBuffer_important; //Fluffy

	if (texture_intermediate)
	    SDL_DestroyTexture(texture_intermediate); //Fluffy

	if (texture_lightmap)
		SDL_DestroyTexture(texture_lightmap); //Fluffy

	if (texture_walls)
		SDL_DestroyTexture(texture_walls); //Fluffy
	if (surface_walls_palette)
		SDL_FreePalette(surface_walls_palette); //Fluffy
	if (surface_walls)
		SDL_FreeSurface(surface_walls); //Fluffy
	if (lightmap_wall_buffer)
		delete[] lightmap_wall_buffer; //Fluffy

	if (ghMainWnd)
		SDL_HideWindow(ghMainWnd);
	sgMemCrit.Enter();
	sgdwLockCount = 0;
	gpBuffer = NULL;
	sgMemCrit.Leave();

	if (pal_surface == NULL)
		return;
	SDL_FreeSurface(pal_surface);
	pal_surface = NULL;
	SDL_FreePalette(palette);
	SDL_FreeSurface(renderer_texture_surface);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(ghMainWnd);
}

void dx_reinit()
{
#ifdef USE_SDL1
	ghMainWnd = SDL_SetVideoMode(0, 0, 0, ghMainWnd->flags ^ SDL_FULLSCREEN);
	if (ghMainWnd == NULL) {
		ErrSdl();
	}
#else
	Uint32 flags = 0;
	if (!fullscreen) {
		flags = renderer ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
	}
	if (SDL_SetWindowFullscreen(ghMainWnd, flags)) {
		ErrSdl();
	}
#endif
	fullscreen = !fullscreen;
	force_redraw = 255;
}

void InitPalette()
{
	palette = SDL_AllocPalette(256);
	if (palette == NULL) {
		ErrSdl();
	}
}

void BltFast(SDL_Rect *src_rect, SDL_Rect *dst_rect)
{
	Blit(pal_surface, src_rect, dst_rect);
}

void Blit(SDL_Surface *src, SDL_Rect *src_rect, SDL_Rect *dst_rect)
{
	SDL_Surface *dst = GetOutputSurface();
#ifndef USE_SDL1
	if (SDL_BlitSurface(src, src_rect, dst, dst_rect) < 0)
			ErrSdl();
		return;
#else
	if (!OutputRequiresScaling()) {
		if (SDL_BlitSurface(src, src_rect, dst, dst_rect) < 0)
			ErrSdl();
		return;
	}

	SDL_Rect scaled_dst_rect;
	if (dst_rect != NULL) {
		scaled_dst_rect = *dst_rect;
		ScaleOutputRect(&scaled_dst_rect);
		dst_rect = &scaled_dst_rect;
	}

	// Same pixel format: We can call BlitScaled directly.
	if (SDLBackport_PixelFormatFormatEq(src->format, dst->format)) {
		if (SDL_BlitScaled(src, src_rect, dst, dst_rect) < 0)
			ErrSdl();
		return;
	}

	// If the surface has a color key, we must stretch first and can then call BlitSurface.
	if (SDL_HasColorKey(src)) {
		SDL_Surface *stretched = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_rect->w, dst_rect->h, src->format->BitsPerPixel,
		    src->format->Rmask, src->format->Gmask, src->format->BitsPerPixel, src->format->Amask);
		SDL_SetColorKey(stretched, SDL_SRCCOLORKEY, src->format->colorkey);
		if (src->format->palette != NULL)
			SDL_SetPalette(stretched, SDL_LOGPAL, src->format->palette->colors, 0, src->format->palette->ncolors);
		SDL_Rect stretched_rect = { 0, 0, dst_rect->w, dst_rect->h };
		if (SDL_SoftStretch(src, src_rect, stretched, &stretched_rect) < 0
		    || SDL_BlitSurface(stretched, &stretched_rect, dst, dst_rect) < 0) {
			SDL_FreeSurface(stretched);
			ErrSdl();
		}
		SDL_FreeSurface(stretched);
		return;
	}

	// A surface with a non-output pixel format but without a color key needs scaling.
	// We can convert the format and then call BlitScaled.
	SDL_Surface *converted = SDL_ConvertSurface(src, dst->format, 0);
	if (SDL_BlitScaled(converted, src_rect, dst, dst_rect) < 0) {
		SDL_FreeSurface(converted);
		ErrSdl();
	}
	SDL_FreeSurface(converted);
#endif
}

/**
 * @brief Limit FPS to avoid high CPU load, use when v-sync isn't available
 */
void LimitFrameRate()
{
	static uint32_t frameDeadline;
	uint32_t tc = SDL_GetTicks() * 1000;
	uint32_t v = 0;
	if (frameDeadline > tc) {
		v = tc % refreshDelay;
		SDL_Delay(v / 1000 + 1); // ceil
	}
	frameDeadline = tc + v + refreshDelay;
}

void RenderPresent()
{
	//Fluffy: Calculate delta between current and previous displayed frame
	{
		unsigned long long curTime = SDL_GetPerformanceCounter();
		if (frame_timeOfPreviousFrameRender != 0)
			frame_renderDelta = (double)((curTime - frame_timeOfPreviousFrameRender) * 1000) / SDL_GetPerformanceFrequency();
		frame_timeOfPreviousFrameRender = curTime;
	}

	SDL_Surface *surface = GetOutputSurface();
	assert(!SDL_MUSTLOCK(surface));

	if (!gbActive) {
		LimitFrameRate();
		return;
	}

#ifndef USE_SDL1
	if (renderer) {
		if (SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch) <= -1) { //pitch is 2560
			ErrSdl();
		}

		// Clear buffer to avoid artifacts in case the window was resized
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) <= -1) { // TODO only do this if window was resized
			ErrSdl();
		}

		if (SDL_RenderClear(renderer) <= -1) {
			ErrSdl();
		}

		if (SDL_RenderCopy(renderer, texture, NULL, NULL) <= -1) {
			ErrSdl();
		}

		if (options_hwRendering) {
			if (options_lightmapping && dx_useLightmap) {
				//Fluffy: Render surface_walls onto texture_lightmap
				{
					for (int x = 0; x < 1280; x++)
						for (int y = 0; y < 720; y++) {
							unsigned int pos = 0;
							pos += BUFFER_WIDTH * (y + BORDER_TOP);
							pos += x + BORDER_LEFT;
							unsigned int toPos = 0;
							toPos += (SCREEN_WIDTH * 4) * (y);
							toPos += x * 4;
							//lightmap_walls[pos] = 128;
							lightmap_wall_buffer[toPos + 3] = lightmap_walls[pos];
							lightmap_wall_buffer[toPos + 2] = lightmap_walls[pos];
							lightmap_wall_buffer[toPos + 1] = lightmap_walls[pos];
							lightmap_wall_buffer[toPos + 0] = 255;
						}
				}

				SDL_UpdateTexture(texture_walls, NULL, lightmap_wall_buffer, SCREEN_WIDTH * 4); //Update walls texture based on walls surface
				SDL_SetRenderTarget(renderer, texture_lightmap); //Set render target to lightmap
				SDL_RenderCopy(renderer, texture_walls, NULL, NULL); //Render walls texture to lightmap
				SDL_SetRenderTarget(renderer, NULL); //Switch back to normal renderer
				
				if (SDL_RenderCopy(renderer, texture_lightmap, NULL, NULL) <= -1) { //Fluffy: Render lightmap for ingame graphics
					ErrSdl();
				}
			}
			if (SDL_RenderCopy(renderer, texture_intermediate, NULL, NULL) <= -1) { //Fluffy: Render intermediate texture
				ErrSdl();
			}
		}

		if (options_hwRendering && dx_fade > 0) { //Fluffy: Render a black rectangle with dx_fadeStatus as alpha value
			if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, dx_fade) < 0)
				ErrSdl();
			if(SDL_RenderFillRect(renderer, NULL) < 0)
				ErrSdl();
		}
		
		SDL_RenderPresent(renderer);

		if (!vsyncEnabled) {
			LimitFrameRate();
		}
	} else {
		if (SDL_UpdateWindowSurface(ghMainWnd) <= -1) {
			ErrSdl();
		}
		LimitFrameRate();
	}
#else
	if (SDL_Flip(surface) <= -1) {
		ErrSdl();
	}
	LimitFrameRate();
#endif
}

void PaletteGetEntries(DWORD dwNumEntries, SDL_Color *lpEntries)
{
	for (DWORD i = 0; i < dwNumEntries; i++) {
		lpEntries[i] = system_palette[i];
	}
}
} // namespace dvl
