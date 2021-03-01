/**
 * @file render.cpp
 *
 * Implementation of functionality for rendering the level tiles.
 */
#include "all.h"
#include "textures/textures.h" //Fluffy: For SDL textures
#include "render/render.h" //Fluffy: For rendering SDL textures

DEVILUTION_BEGIN_NAMESPACE

#define NO_OVERDRAW

enum {
	RT_SQUARE,
	RT_TRANSPARENT,
	RT_LTRIANGLE,
	RT_RTRIANGLE,
	RT_LTRAPEZOID,
	RT_RTRAPEZOID
};

/** Fluffy: Fully transparent variant of WallMask. */
static DWORD WallMask_FullyTransparent[TILE_HEIGHT] = {
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000
};

/** Fluffy: Transparent variant of RightMask. */
static DWORD RightMask_Transparent[TILE_HEIGHT] = {
	0xE0000000, 0xF0000000,
	0xFE000000, 0xFF000000,
	0xFFE00000, 0xFFF00000,
	0xFFFE0000, 0xFFFF0000,
	0xFFFFE000, 0xFFFFF000,
	0xFFFFFE00, 0xFFFFFF00,
	0xFFFFFFE0, 0xFFFFFFF0,
	0xFFFFFFFE, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};
/** Fluffy: Transparent variant of LeftMask. */
static DWORD LeftMask_Transparent[TILE_HEIGHT] = {
	0x00000003, 0x0000000F,
	0x0000003F, 0x000000FF,
	0x000003FF, 0x00000FFF,
	0x00003FFF, 0x0000FFFF,
	0x0003FFFF, 0x000FFFFF,
	0x003FFFFF, 0x00FFFFFF,
	0x03FFFFFF, 0x0FFFFFFF,
	0x3FFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};

/** Specifies the draw masks used to render transparency of the right side of tiles. */
static DWORD RightMask[TILE_HEIGHT] = {
	0xEAAAAAAA, 0xF5555555,
	0xFEAAAAAA, 0xFF555555,
	0xFFEAAAAA, 0xFFF55555,
	0xFFFEAAAA, 0xFFFF5555,
	0xFFFFEAAA, 0xFFFFF555,
	0xFFFFFEAA, 0xFFFFFF55,
	0xFFFFFFEA, 0xFFFFFFF5,
	0xFFFFFFFE, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of the left side of tiles. */
static DWORD LeftMask[TILE_HEIGHT] = {
	0xAAAAAAAB, 0x5555555F,
	0xAAAAAABF, 0x555555FF,
	0xAAAAABFF, 0x55555FFF,
	0xAAAABFFF, 0x5555FFFF,
	0xAAABFFFF, 0x555FFFFF,
	0xAABFFFFF, 0x55FFFFFF,
	0xABFFFFFF, 0x5FFFFFFF,
	0xBFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of wall tiles. */
static DWORD WallMask[TILE_HEIGHT] = {
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555
};

static DWORD SolidMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};

static DWORD RightFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0x3FFFFFFF,
	0x0FFFFFFF, 0x03FFFFFF,
	0x00FFFFFF, 0x003FFFFF,
	0x000FFFFF, 0x0003FFFF,
	0x0000FFFF, 0x00003FFF,
	0x00000FFF, 0x000003FF,
	0x000000FF, 0x0000003F,
	0x0000000F, 0x00000003,
	0x00000000, 0x00000003,
	0x0000000F, 0x0000003F,
	0x000000FF, 0x000003FF,
	0x00000FFF, 0x00003FFF,
	0x0000FFFF, 0x0003FFFF,
	0x000FFFFF, 0x003FFFFF,
	0x00FFFFFF, 0x03FFFFFF,
	0x0FFFFFFF, 0x3FFFFFFF,
};

static DWORD LeftFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0xFFFFFFFC,
	0xFFFFFFF0, 0xFFFFFFC0,
	0xFFFFFF00, 0xFFFFFC00,
	0xFFFFF000, 0xFFFFC000,
	0xFFFF0000, 0xFFFC0000,
	0xFFF00000, 0xFFC00000,
	0xFF000000, 0xFC000000,
	0xF0000000, 0xC0000000,
	0x00000000, 0xC0000000,
	0xF0000000, 0xFC000000,
	0xFF000000, 0xFFC00000,
	0xFFF00000, 0xFFFC0000,
	0xFFFF0000, 0xFFFFC000,
	0xFFFFF000, 0xFFFFFC00,
	0xFFFFFF00, 0xFFFFFFC0,
	0xFFFFFFF0, 0xFFFFFFFC,
};

inline static int count_leading_zeros(DWORD mask)
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

inline static void RenderLine(BYTE **dst, BYTE **src, int n, BYTE *tbl, DWORD mask)
{
#ifdef NO_OVERDRAW
	if (*dst < gpBufStart || *dst > gpBufEnd) {
		goto skip;
	}
#endif

	BYTE *importantBuff; //Fluffy: For wall transparency. (Only used if certain toggles are on)
	if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
		importantBuff = gpBuffer_important + ((*dst) - gpBuffer);

	if (mask == 0xFFFFFFFF) { //Opaque line
		if (light_table_index == lightmax) { //Complete darkness
			memset(*dst, 0, n);
		} else if (light_table_index == 0) { //Fully lit
			memcpy(*dst, *src, n);
		} else { //Partially lit
			for (int i = 0; i < n; i++) {
				(*dst)[i] = tbl[(*src)[i]];
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

		if (options_transparency) { //Render transparent pixels in the mask with actual transparent, and the rest as opaque pixels
			if (light_table_index == lightmax) { //Complete darkness
				for (int i = 0; i < n; i++, mask <<= 1) {
					if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
						(*dst)[i] = palette_transparency_lookup[0][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
					else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
						(*dst)[i] = 0;
					else if (options_transparency || (options_opaqueWallsWithBlobs && *importantBuff == 1))
						(*dst)[i] = palette_transparency_lookup[0][(*dst)[i]];

					if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
						importantBuff++;
				}
			} else if (light_table_index == 0) { //Fully lit
				for (int i = 0; i < n; i++, mask <<= 1) {
					if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
						(*dst)[i] = palette_transparency_lookup[(*src)[i]][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
					else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
						(*dst)[i] = (*src)[i];
					else if (options_transparency || (options_opaqueWallsWithBlobs && *importantBuff == 1))
						(*dst)[i] = palette_transparency_lookup[(*dst)[i]][(*src)[i]];

					if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
						importantBuff++;
				}
			} else { //Partially lit
				for (int i = 0; i < n; i++, mask <<= 1) {
					if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
						(*dst)[i] = palette_transparency_lookup[tbl[(*src)[i]]][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
					else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
						(*dst)[i] = tbl[(*src)[i]];
					else if (options_transparency || (options_opaqueWallsWithBlobs && *importantBuff == 1))
						(*dst)[i] = palette_transparency_lookup[(*dst)[i]][tbl[(*src)[i]]];

					if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
						importantBuff++;
				}
			}
		} else { //Default Diablo 1 rendering where transparent pixels are skipped
			if (light_table_index == lightmax) { //Complete darkness
				foreach_set_bit(mask, [=](int i) { (*dst)[i] = 0; });
			} else if (light_table_index == 0) { //Fully lit
				foreach_set_bit(mask, [=](int i) { (*dst)[i] = (*src)[i]; });
			} else { //Partially lit
				foreach_set_bit(mask, [=](int i) { (*dst)[i] = tbl[(*src)[i]]; });
			}
		}
	}

skip:
	(*src) += n;
	(*dst) += n;
}

void RenderTileViaSDL(int sx, int sy)
{
	int frame = (level_cel_block & 0xFFF) - 1;
	int brightness = 255 - ((light_table_index * 255) / lightmax);
	SDL_SetTextureColorMod(textures[TEXTURE_DUNGEONTILES].frames[frame].frame, brightness, brightness, brightness);
	if (arch_draw_type == 0 && cel_transparency_active)
		SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES].frames[frame].frame, 127);
	Render_Texture_FromBottomLeft(sx - BORDER_LEFT, sy - BORDER_TOP, TEXTURE_DUNGEONTILES, frame);
	SDL_SetTextureColorMod(textures[TEXTURE_DUNGEONTILES].frames[frame].frame, 255, 255, 255);
	if (arch_draw_type == 0 && cel_transparency_active)
		SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES].frames[frame].frame, 255);
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
/**
 * @brief Blit current world CEL to the given buffer
 * @param pBuff Output buffer
 */
void RenderTile(BYTE *pBuff)
{
	int i, j;
	char c, v, tile;
	BYTE *src, *dst, *tbl;
	DWORD m, *mask, *pFrameTable;

	dst = pBuff;
	pFrameTable = (DWORD *)pDungeonCels;

	src = &pDungeonCels[SDL_SwapLE32(pFrameTable[level_cel_block & 0xFFF])];
	tile = (level_cel_block & 0x7000) >> 12;
	tbl = &pLightTbl[256 * light_table_index];

	//The mask defines what parts of the tile is opaque
	mask = &SolidMask[TILE_HEIGHT - 1];
	if (cel_transparency_active) {
		if (arch_draw_type == 0) {
			if (options_transparency == 1) //Fluffy
				mask = &WallMask_FullyTransparent[TILE_HEIGHT - 1];
			else
				mask = &WallMask[TILE_HEIGHT - 1];
		}
		if (arch_draw_type == 1 && tile != RT_LTRIANGLE) {
			c = block_lvid[level_piece_id];
			if (c == 1 || c == 3) {
				if (options_transparency == 1) //Fluffy
					mask = &LeftMask_Transparent[TILE_HEIGHT - 1];
				else
					mask = &LeftMask[TILE_HEIGHT - 1];
			}
		}
		if (arch_draw_type == 2 && tile != RT_RTRIANGLE) {
			c = block_lvid[level_piece_id];
			if (c == 2 || c == 3) {
				if (options_transparency == 1) //Fluffy
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

	switch (tile) {
	case RT_SQUARE: //Draws a 32x32 square. This is used for walls
		for (i = TILE_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(&dst, &src, TILE_WIDTH / 2, tbl, *mask);
		}
		break;
	case RT_TRANSPARENT: //I've seen this be used for tiles with rubble on them (the rubble extends upwards beyond the normal boundaries of the tile). I've also seen it be used by top parts of walls, the tiles containing bottom part of torches, and pillars
		for (i = TILE_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			m = *mask;
			for (j = TILE_WIDTH / 2; j != 0; j -= v, v == TILE_WIDTH / 2 ? m = 0 : m <<= v) {
				v = *src++;
				if (v >= 0) {
					RenderLine(&dst, &src, v, tbl, m);
				} else {
					v = -v;
					dst += v;
				}
			}
		}
		break;
	case RT_LTRIANGLE: //This is used for 99% of floor tiles
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) { //Bottomleft
			src += i & 2;
			dst += i;
			RenderLine(&dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
		}
		for (i = 2; i != TILE_WIDTH / 2; i += 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) { //Topleft
			src += i & 2;
			dst += i;
			RenderLine(&dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
		}
		break;
	case RT_RTRIANGLE: //This is used for 99% of floor tiles
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) { //Bottomright
			RenderLine(&dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
			src += i & 2;
			dst += i;
		} 
		for (i = 2; i != TILE_HEIGHT; i += 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) { //Topright
			RenderLine(&dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
			src += i & 2;
			dst += i;
		}
		break;
	case RT_LTRAPEZOID: //Used for parts of walls
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(&dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
		}
		for (i = TILE_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(&dst, &src, TILE_WIDTH / 2, tbl, *mask);
		}
		break;
	case RT_RTRAPEZOID: //Used for parts of walls
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(&dst, &src, TILE_WIDTH / 2 - i, tbl, *mask);
			src += i & 2;
			dst += i;
		}
		for (i = TILE_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(&dst, &src, TILE_WIDTH / 2, tbl, *mask);
		}
		break;
	}
}

/**
 * @brief Render a black tile
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void world_draw_black_tile(int sx, int sy)
{
	int i, j, k;
	BYTE *dst;

	if (sx >= SCREEN_X + SCREEN_WIDTH || sy >= SCREEN_Y + VIEWPORT_HEIGHT + TILE_WIDTH / 2)
		return;

	if (sx < SCREEN_X - (TILE_WIDTH - 4) || sy < SCREEN_Y)
		return;

	dst = &gpBuffer[sx + BUFFER_WIDTH * sy] + TILE_WIDTH / 2 - 2;

	for (i = TILE_HEIGHT - 2, j = 1; i >= 0; i -= 2, j++, dst -= BUFFER_WIDTH + 2) {
		if (dst < gpBufEnd)
			memset(dst, 0, 4 * j);
	}
	dst += 4;
	for (i = 2, j = TILE_HEIGHT / 2 - 1; i != TILE_HEIGHT; i += 2, j--, dst -= BUFFER_WIDTH - 2) {
		if (dst < gpBufEnd)
			memset(dst, 0, 4 * j);
	}
}

/**
 * Draws a half-transparent rectangle by blacking out odd pixels on odd lines,
 * even pixels on even lines.
 * @brief Render a transparent black rectangle
 * @param sx Screen coordinate
 * @param sy Screen coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 */
void trans_rect(int sx, int sy, int width, int height)
{
	int row, col;
	BYTE *pix = &gpBuffer[SCREENXY(sx, sy)];
	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			if (options_transparency == 1) //Fluffy
				*pix = palette_transparency_lookup[0][*pix];
			else if ((row & 1 && col & 1) || (!(row & 1) && !(col & 1)))
				*pix = 0;
			pix++;
		}
		pix += BUFFER_WIDTH - width;
	}
}

DEVILUTION_END_NAMESPACE
