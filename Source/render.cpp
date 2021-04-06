/**
 * @file render.cpp
 *
 * Implementation of functionality for rendering the level tiles.
 */
#include "all.h"
#include "options.h"
#include "textures/textures.h" //Fluffy: For SDL textures
#include "render/sdl-render.h" //Fluffy: For rendering SDL textures
#include "render/lightmap.h" //Fluffy: For getting light values from the lightmap

DEVILUTION_BEGIN_NAMESPACE

#define NO_OVERDRAW

namespace {

enum {
	RT_SQUARE,
	RT_TRANSPARENT,
	RT_LTRIANGLE,
	RT_RTRIANGLE,
	RT_LTRAPEZOID,
	RT_RTRAPEZOID
};

/** Fully transparent variant of WallMask. */
const DWORD WallMask_FullyTrasparent[TILE_HEIGHT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};
/** Transparent variant of RightMask. */
const DWORD RightMask_Transparent[TILE_HEIGHT] = {
	0xC0000000,
	0xF0000000,
	0xFC000000,
	0xFF000000,
	0xFFC00000,
	0xFFF00000,
	0xFFFC0000,
	0xFFFF0000,
	0xFFFFC000,
	0xFFFFF000,
	0xFFFFFC00,
	0xFFFFFF00,
	0xFFFFFFC0,
	0xFFFFFFF0,
	0xFFFFFFFC,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF
};
/** Transparent variant of LeftMask. */
const DWORD LeftMask_Transparent[TILE_HEIGHT] = {
	0x00000003,
	0x0000000F,
	0x0000003F,
	0x000000FF,
	0x000003FF,
	0x00000FFF,
	0x00003FFF,
	0x0000FFFF,
	0x0003FFFF,
	0x000FFFFF,
	0x003FFFFF,
	0x00FFFFFF,
	0x03FFFFFF,
	0x0FFFFFFF,
	0x3FFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of the right side of tiles. */
const DWORD RightMask[TILE_HEIGHT] = {
	0xEAAAAAAA,
	0xF5555555,
	0xFEAAAAAA,
	0xFF555555,
	0xFFEAAAAA,
	0xFFF55555,
	0xFFFEAAAA,
	0xFFFF5555,
	0xFFFFEAAA,
	0xFFFFF555,
	0xFFFFFEAA,
	0xFFFFFF55,
	0xFFFFFFEA,
	0xFFFFFFF5,
	0xFFFFFFFE,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of the left side of tiles. */
const DWORD LeftMask[TILE_HEIGHT] = {
	0xAAAAAAAB,
	0x5555555F,
	0xAAAAAABF,
	0x555555FF,
	0xAAAAABFF,
	0x55555FFF,
	0xAAAABFFF,
	0x5555FFFF,
	0xAAABFFFF,
	0x555FFFFF,
	0xAABFFFFF,
	0x55FFFFFF,
	0xABFFFFFF,
	0x5FFFFFFF,
	0xBFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of wall tiles. */
const DWORD WallMask[TILE_HEIGHT] = {
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555,
	0xAAAAAAAA,
	0x55555555
};
/** Fully opaque mask */
const DWORD SolidMask[TILE_HEIGHT] = {
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF
};
/** Used to mask out the left half of the tile diamond and only render additional content */
const DWORD RightFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF,
	0x3FFFFFFF,
	0x0FFFFFFF,
	0x03FFFFFF,
	0x00FFFFFF,
	0x003FFFFF,
	0x000FFFFF,
	0x0003FFFF,
	0x0000FFFF,
	0x00003FFF,
	0x00000FFF,
	0x000003FF,
	0x000000FF,
	0x0000003F,
	0x0000000F,
	0x00000003,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};
/** Used to mask out the left half of the tile diamond and only render additional content */
const DWORD LeftFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF,
	0xFFFFFFFC,
	0xFFFFFFF0,
	0xFFFFFFC0,
	0xFFFFFF00,
	0xFFFFFC00,
	0xFFFFF000,
	0xFFFFC000,
	0xFFFF0000,
	0xFFFC0000,
	0xFFF00000,
	0xFFC00000,
	0xFF000000,
	0xFC000000,
	0xF0000000,
	0xC0000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

inline int count_leading_zeros(DWORD mask)
{
	// Note: This function assumes that the argument is not zero,
	// which means there is at least one bit set.
	static_assert(
	    sizeof(DWORD) == sizeof(uint32_t),
	    "count_leading_zeros: DWORD must be 32bits");
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_clz(mask);
#else
	// Count the number of leading zeros using binary search.
	int n = 0;
	if ((mask & 0xFFFF0000) == 0)
		n += 16, mask <<= 16;
	if ((mask & 0xFF000000) == 0)
		n += 8, mask <<= 8;
	if ((mask & 0xF0000000) == 0)
		n += 4, mask <<= 4;
	if ((mask & 0xC0000000) == 0)
		n += 2, mask <<= 2;
	if ((mask & 0x80000000) == 0)
		n += 1;
	return n;
#endif
}

template <typename F>
void foreach_set_bit(DWORD mask, const F &f)
{
	int i = 0;
	while (mask != 0) {
		int z = count_leading_zeros(mask);
		i += z, mask <<= z;
		for (; mask & 0x80000000; i++, mask <<= 1)
			f(i);
	}
}

inline void DoRenderLine(BYTE *dst, BYTE *src, int n, BYTE *tbl, DWORD mask)
{
	BYTE *importantBuff; //Fluffy: For wall transparency. (Only used if certain toggles are on)
	//Fluffy TODO Merge: Rewrite importantBuff code so it works again
	/*
	if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
		importantBuff = gpBuffer_important + ((*dst) - gpBuffer);*/
	if (mask == 0xFFFFFFFF) {                // Opaque line
		if (light_table_index == lightmax) { // Complete darkness
			memset(dst, 0, n);
		} else if (light_table_index == 0) { // Fully lit
			memcpy(dst, src, n);
		} else { // Partially lit
			for (int i = 0; i < n; i++) {
				dst[i] = tbl[src[i]];
			}
		}

		if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
			importantBuff += n; //Fluffy
	} else {
		// The number of iterations is anyway limited by the size of the mask.
		// So we can limit it by ANDing the mask with another mask that only keeps
		// iterations that are lower than n. We can now avoid testing if i < n
		// at every loop iteration.
		assert(n != 0 && n <= sizeof(DWORD) * CHAR_BIT);
		mask &= DWORD(-1) << ((sizeof(DWORD) * CHAR_BIT) - n);

		if (sgOptions.Graphics.bBlendedTransparancy || options_opaqueWallsWithSilhouette || options_opaqueWallsWithBlobs) { // Blended transparancy
			if (light_table_index == lightmax) {       // Complete darkness
				for (int i = 0; i < n; i++, mask <<= 1) {
					if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
						dst[i] = paletteTransparencyLookup[0][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
					else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
						dst[i] = 0;
					else if (sgOptions.Graphics.bBlendedTransparancy || (options_opaqueWallsWithBlobs && *importantBuff == 1))
						dst[i] = paletteTransparencyLookup[0][dst[i]];

					if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
						importantBuff++;
				}
			} else if (light_table_index == 0) { //Fully lit
				for (int i = 0; i < n; i++, mask <<= 1) {
					if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
						dst[i] = paletteTransparencyLookup[src[i]][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
					else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
						dst[i] = src[i];
					else if (sgOptions.Graphics.bBlendedTransparancy || (options_opaqueWallsWithBlobs && *importantBuff == 1))
						dst[i] = paletteTransparencyLookup[dst[i]][src[i]];

					if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
						importantBuff++;
				}
			} else { //Partially lit
				for (int i = 0; i < n; i++, mask <<= 1) {
					if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
						dst[i] = paletteTransparencyLookup[tbl[src[i]]][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
					else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
						dst[i] = tbl[src[i]];
					else if (sgOptions.Graphics.bBlendedTransparancy || (options_opaqueWallsWithBlobs && *importantBuff == 1))
						dst[i] = paletteTransparencyLookup[dst[i]][tbl[src[i]]];

					if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
						importantBuff++;
				}
			}
		} else {                                 // Stippled transparancy
			if (light_table_index == lightmax) { // Complete darkness
				foreach_set_bit(mask, [=](int i) { dst[i] = 0; });
			} else if (light_table_index == 0) { // Fully lit
				foreach_set_bit(mask, [=](int i) { dst[i] = src[i]; });
			} else { // Partially lit
				foreach_set_bit(mask, [=](int i) { dst[i] = tbl[src[i]]; });
			}
		}
	}
}

DVL_ATTRIBUTE_ALWAYS_INLINE
inline void RenderLine(BYTE *dst_begin, BYTE *dst_end, BYTE **dst, BYTE **src, int n, BYTE *tbl, DWORD mask)
{
#ifdef NO_OVERDRAW
	if (*dst >= dst_begin && *dst <= dst_end)
#endif
		DoRenderLine(*dst, *src, n, tbl, mask);
skip:
	(*src) += n;
	(*dst) += n;
}

} // namespace

void RenderTileViaSDL(int sx, int sy, int lightx, int lighty, int lightType)
{
	int frame = (level_cel_block & 0xFFF) - 1;
	int brightness;
	int tile = (level_cel_block & 0xF000) >> 12;
	int overlayTexture = -1;
	bool repeatRender = false; //Used for tiles containing both ceiling and wall image data
	int dungeonTilesTexture = TEXTURE_DUNGEONTILES;

repeat:
	if (options_lightmapping) { //Ceiling tiles get lightmapping applied in a unique way
		if (leveltype == DTYPE_CATHEDRAL && currlevel < 21) { //Cathedral
			if (frame == 113 || frame == 114 || frame == 115 || frame == 116 || frame == 118 || frame == 121 || frame == 124 || frame == 125 || frame == 129 || frame == 132 || (frame >= 1099 && frame <= 1104)) { //Tiles that are 100% ceiling
				lightType = LIGHTING_SUBTILE_LIGHTMAP;
			}

			//Tiles that are a mix of ceiling and wall
			if (frame == 112 || frame == 117 || frame == 126 || frame == 127) //Left mask
			{
				if (repeatRender) {
					lightType = LIGHTING_SUBTILE_LIGHTMAP;
					dungeonTilesTexture = TEXTURE_DUNGEONTILES_LEFTMASKOPAQUE;
					repeatRender = false;
				} else {
					dungeonTilesTexture = TEXTURE_DUNGEONTILES_LEFTMASKINVERTED;
					repeatRender = true;
				}
			} else if (frame == 119 || frame == 120 || frame == 133) //Right mask
			{
				if (repeatRender) {
					lightType = LIGHTING_SUBTILE_LIGHTMAP;
					dungeonTilesTexture = TEXTURE_DUNGEONTILES_RIGHTMASKOPAQUE;
					repeatRender = false;
				} else {
					dungeonTilesTexture = TEXTURE_DUNGEONTILES_RIGHTMASKINVERTED;
					repeatRender = true;
				}
			}
		}
	}

	bool transparent = false;
#ifndef _DEBUG
	if (cel_transparency_active) {
#else
	if (cel_transparency_active && !(GetAsyncKeyState(DVL_VK_MENU) & 0x8000)) {
		if (arch_draw_type && cel_foliage_active && tile != RT_TRANSPARENT) {
			return;
		}
#endif

		if (arch_draw_type == 0)
			transparent = true;
		else if (arch_draw_type == 1 && tile != RT_LTRIANGLE) {
			dungeonTilesTexture = TEXTURE_DUNGEONTILES_LEFTMASK;
		} else if (arch_draw_type == 2 && tile != RT_RTRIANGLE) {
			dungeonTilesTexture = TEXTURE_DUNGEONTILES_RIGHTMASK;
		}
	} else if (arch_draw_type && cel_foliage_active) {
		if (tile != RT_TRANSPARENT) {
			return;
		}
		if (arch_draw_type == 1) {
			dungeonTilesTexture = TEXTURE_DUNGEONTILES_LEFTFOLIAGE;
		} else if (arch_draw_type == 2) {
			dungeonTilesTexture = TEXTURE_DUNGEONTILES_RIGHTFOLIAGE;
		}
	}

	SDL_Texture *tex = textures[dungeonTilesTexture].frames[0].frame;
	if (transparent)
		SDL_SetTextureAlphaMod(tex, 191);

	//if (options_lightmapping && arch_draw_type != 0) { //Optimization check. If this part of the lightmap is 100% dark (and not transparent), then don't render this tile
	//	if (lightType == LIGHTING_SUBTILE_LIGHTMAP) { //We check the four corners of this lightmap piece
	//		int x = sx - BORDER_LEFT;
	//		int y = (sy - BORDER_TOP) - (32 - 1);
	//		y += 64;
	//		if (x + 32 >= gnScreenWidth || y + 32 >= gnScreenHeight) {
	//			if (repeatRender)
	//				goto repeat;
	//			return;
	//		}
	//		if (Lightmap_ReturnBrightness(x, y) == 0 && Lightmap_ReturnBrightness(x + 31, y) == 0
	//			&& Lightmap_ReturnBrightness(x + 31, y + 31) == 0 && Lightmap_ReturnBrightness(x, y + 31) == 0) {
	//			if (repeatRender)
	//				goto repeat;
	//			return;
	//		}
	//	} else if (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD || lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD) { //We check start and end points of a line on the lightmap
	//		int x = sx - BORDER_LEFT;
	//		int y = sy - BORDER_TOP;
	//		if (Lightmap_ReturnBrightness(x, y) == 0 && Lightmap_ReturnBrightness(x + 31, y) == 0) {
	//			if (repeatRender)
	//				goto repeat;
	//			return;
	//		}
	//	}
	//}

	int textureNum = dungeonTilesTexture;
	if (lightType != LIGHTING_SUBTILE_NONE) { //Fluffy: We take the pixel at coordinates lightx and lighty from the lightmap and apply that to the entire texture
		if (lightType == LIGHTING_SUBTILE_LIGHTMAP) {
			//Switch to the intermediate tile render target
			SDL_SetRenderTarget(renderer, textures[TEXTURE_TILE_INTERMEDIATE].frames[0].frame);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE); //Switch to "none" blend mode so we overwrite everything in the render target
		}

		SDL_Rect dstRect;
		if (lightType == LIGHTING_SUBTILE_LIGHTMAP) {
			dstRect.x = 0;
			dstRect.y = 0;
		} else {
			dstRect.x = sx - BUFFER_BORDER_LEFT;
			dstRect.y = (sy - BUFFER_BORDER_TOP) - (textures[textureNum].frames[frame].height - 1);
		}
		dstRect.w = 1;
		dstRect.h = textures[textureNum].frames[frame].height;

		if (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD || lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD) {
			SDL_Rect srcRect;
			srcRect.x = 0;
			srcRect.y = 0;
			srcRect.w = 1;
			srcRect.h = textures[textureNum].frames[frame].height;
			if (textures[textureNum].usesAtlas) {
				srcRect.x = textures[textureNum].frames[frame].offsetX;
				srcRect.y = textures[textureNum].frames[frame].offsetY;
			}
			
			for (int i = 0; i < textures[textureNum].frames[frame].width; i++) {
				brightness = Lightmap_ReturnBrightness(lightx, lighty);
				SDL_SetTextureColorMod(tex, brightness, brightness, brightness);
				SDL_RenderCopy(renderer, tex, &srcRect, &dstRect);
				dstRect.x += 1;
				srcRect.x += 1;
				lightx += 1;
				if (i > 0 && i % 2 == 0) {
					if (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD) {
						lighty -= 1;
					} else if (lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD) {
						lighty += 1;
					}
				}
			}
		} else {
			if (lightType != LIGHTING_SUBTILE_LIGHTMAP) {
				brightness = Lightmap_ReturnBrightness(lightx, lighty);
				SDL_SetTextureColorMod(tex, brightness, brightness, brightness);
			}
			Render_Texture(dstRect.x, dstRect.y, textureNum, frame);
		}
		SDL_SetTextureColorMod(tex, 255, 255, 255);
		if (transparent)
			SDL_SetTextureAlphaMod(tex, 255);

		if (lightType == LIGHTING_SUBTILE_LIGHTMAP) {
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND); //Revert blend mode for the texture
			if (lightType == LIGHTING_SUBTILE_LIGHTMAP) {
				int x = sx - BUFFER_BORDER_LEFT;
				int y = (sy - BUFFER_BORDER_TOP) - (textures[textureNum].frames[frame].height - 1);
				y += 64;
				if (x < 0)
					x = 0;
				if (y < 0)
					y = 0;
				Render_Texture_Crop(0, 0, TEXTURE_LIGHT_FRAMEBUFFER, x, y, x + 32, y + 32);
			}

			//Switch render target back to intermediate texture and render final result
			SDL_SetRenderTarget(renderer, texture_intermediate);
			Render_Texture_FromBottom(sx - BUFFER_BORDER_LEFT, sy - BUFFER_BORDER_TOP, TEXTURE_TILE_INTERMEDIATE, 0);
		}

		if (repeatRender)
			goto repeat;
		return;
	}

	brightness = Render_IndexLightToBrightness();
	SDL_SetTextureColorMod(tex, brightness, brightness, brightness);
	Render_Texture_FromBottom(sx - BUFFER_BORDER_LEFT, sy - BUFFER_BORDER_TOP, textureNum, frame);
	SDL_SetTextureColorMod(tex, 255, 255, 255);
	if (transparent)
		SDL_SetTextureAlphaMod(tex, 255);
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif

void RenderTile(CelOutputBuffer out, int x, int y)
{
	int i, j;
	char c, v, tile;
	BYTE *src, *tbl;
	DWORD m, *pFrameTable;
	const DWORD *mask;

	// TODO: Get rid of overdraw by rendering edge tiles separately.
	out.region.x -= BUFFER_BORDER_LEFT;
	out.region.y -= BUFFER_BORDER_TOP;
	out.region.w += BUFFER_BORDER_LEFT;
	out.region.h += BUFFER_BORDER_TOP;
	x += BUFFER_BORDER_LEFT;
	y += BUFFER_BORDER_TOP;

	pFrameTable = (DWORD *)pDungeonCels;

	src = &pDungeonCels[SDL_SwapLE32(pFrameTable[level_cel_block & 0xFFF])];
	tile = (level_cel_block & 0x7000) >> 12;
	tbl = &pLightTbl[256 * light_table_index];

	// The mask defines what parts of the tile is opaque
	mask = &SolidMask[TILE_HEIGHT - 1];

	if (cel_transparency_active) {
		if (arch_draw_type == 0) {
			if (sgOptions.Graphics.bBlendedTransparancy) // Use a fully transparent mask
				mask = &WallMask_FullyTrasparent[TILE_HEIGHT - 1];
			else
				mask = &WallMask[TILE_HEIGHT - 1];
		}
		if (arch_draw_type == 1 && tile != RT_LTRIANGLE) {
			c = block_lvid[level_piece_id];
			if (c == 1 || c == 3) {
				if (sgOptions.Graphics.bBlendedTransparancy) // Use a fully transparent mask
					mask = &LeftMask_Transparent[TILE_HEIGHT - 1];
				else
					mask = &LeftMask[TILE_HEIGHT - 1];
			}
		}
		if (arch_draw_type == 2 && tile != RT_RTRIANGLE) {
			c = block_lvid[level_piece_id];
			if (c == 2 || c == 3) {
				if (sgOptions.Graphics.bBlendedTransparancy) // Use a fully transparent mask
					mask = &RightMask_Transparent[TILE_HEIGHT - 1];
				else
					mask = &RightMask[TILE_HEIGHT - 1];
			}
		}
	} else if (arch_draw_type && cel_foliage_active) {
		if (tile != RT_TRANSPARENT) {
			return;
		}
		if (arch_draw_type == 1) {
			mask = &LeftFoliageMask[TILE_HEIGHT - 1];
		}
		if (arch_draw_type == 2) {
			mask = &RightFoliageMask[TILE_HEIGHT - 1];
		}
	}

#ifdef _DEBUG
	if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000) {
		mask = &SolidMask[TILE_HEIGHT - 1];
	}
#endif

	BYTE *dst_begin = out.at(0, BUFFER_BORDER_TOP);
	BYTE *dst_end = out.end();
	BYTE *dst = out.at(x, y);
	const int dst_pitch = out.pitch();
	switch (tile) {
	case RT_SQUARE:
		for (i = TILE_HEIGHT; i != 0; i--, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2, tbl, *mask);
		}
		break;
	case RT_TRANSPARENT:
		for (i = TILE_HEIGHT; i != 0; i--, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			m = *mask;
			for (j = TILE_WIDTH / 2; j != 0; j -= v, v == TILE_WIDTH / 2 ? m = 0 : m <<= v) {
				v = *src++;
				if (v >= 0) {
					RenderLine(dst_begin, dst_end, &dst, &src, v, tbl, m);
				} else {
					v = -v;
					dst += v;
				}
			}
		}
		break;
	case RT_LTRIANGLE:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
		}
		for (i = 2; i != TILE_WIDTH / 2; i += 2, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
		}
		break;
	case RT_RTRIANGLE:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
			src += i & 2;
			dst += i;
		}
		for (i = 2; i != TILE_HEIGHT; i += 2, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
			src += i & 2;
			dst += i;
		}
		break;
	case RT_LTRAPEZOID:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
		}
		for (i = TILE_HEIGHT / 2; i != 0; i--, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2, tbl, *mask);
		}
		break;
	case RT_RTRAPEZOID:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
			src += i & 2;
			dst += i;
		}
		for (i = TILE_HEIGHT / 2; i != 0; i--, dst -= dst_pitch + TILE_WIDTH / 2, mask--) {
			RenderLine(dst_begin, dst_end, &dst, &src, TILE_WIDTH / 2, tbl, *mask);
		}
		break;
	}
}

void world_draw_black_tile(CelOutputBuffer out, int sx, int sy)
{
	int i, j;

	if (sx >= gnScreenWidth || sy >= gnViewportHeight + TILE_WIDTH / 2)
		return;

	if (sx < -(TILE_WIDTH - 4) || sy < 0)
		return;

	// TODO: Get rid of overdraw by rendering edge tiles separately.
	out.region.x -= BUFFER_BORDER_LEFT;
	out.region.y -= BUFFER_BORDER_TOP;
	out.region.w += BUFFER_BORDER_LEFT;
	out.region.h += BUFFER_BORDER_TOP;
	sx += BUFFER_BORDER_LEFT;
	sy += BUFFER_BORDER_TOP;

	BYTE *dst = out.at(sx + TILE_WIDTH / 2 - 2, sy);
	for (i = TILE_HEIGHT - 2, j = 1; i >= 0; i -= 2, j++, dst -= out.pitch() + 2) {
		if (dst < out.end())
			memset(dst, 0, 4 * j);
	}
	dst += 4;
	for (i = 2, j = TILE_HEIGHT / 2 - 1; i != TILE_HEIGHT; i += 2, j--, dst -= out.pitch() - 2) {
		if (dst < out.end())
			memset(dst, 0, 4 * j);
	}
}

DEVILUTION_END_NAMESPACE
