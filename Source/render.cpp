/**
 * @file render.cpp
 *
 * Implementation of functionality for rendering the level tiles.
 */
#include "all.h"

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

inline static void RenderLine(BYTE **dst, BYTE **src, int n, BYTE *tbl, DWORD mask) //Draw a horizontal line
{
	int i;

#ifdef NO_OVERDRAW
	if (*dst < gpBufStart || *dst > gpBufEnd) {
		*src += n;
		*dst += n;
		return;
	}
#endif

	BYTE *importantBuff; //Fluffy: For wall transparency. (Only used if certain toggles are on)
	if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
		importantBuff = gpBuffer_important + ((*dst) - gpBuffer);

	if (mask == 0xFFFFFFFF) { //Fully opaque
		if (light_table_index == lightmax) {
			memset(*dst, 0, n); //Render the full line as darkness
			(*src) += n;
			(*dst) += n;
		} else if (light_table_index == 0) {
			memcpy(*dst, *src, n); //Render the full line fully lit
			(*src) += n;
			(*dst) += n;
		} else {
			for (i = 0; i < n; i++, (*src)++, (*dst)++) {
				(*dst)[0] = tbl[(*src)[0]]; //Draw pixels partially lit
			}
		}
		if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
			importantBuff += n; //Fluffy
	} else { //Draw based on mask (if options_transparency is true, then we draw proper transparency on masked pixels. By default this would be dithering)
		if (light_table_index == lightmax) {
			(*src) += n;
			for (i = 0; i < n; i++, (*dst)++, mask <<= 1) {
				if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
					(*dst)[0] = palette_transparency_lookup[0][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
				else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
					(*dst)[0] = 0; //Draw completely black pixel
				else if (options_transparency || (options_opaqueWallsWithBlobs && *importantBuff == 1))
					(*dst)[0] = palette_transparency_lookup[0][(*dst)[0]]; //Fluffy: Transparency

				if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
					importantBuff++;
			}
		} else if (light_table_index == 0) {
			for (i = 0; i < n; i++, (*src)++, (*dst)++, mask <<= 1) {
				if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
					(*dst)[0] = palette_transparency_lookup[(*src)[0]][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
				else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
					(*dst)[0] = (*src)[0]; //Draw fully lit pixel
				else if (options_transparency || (options_opaqueWallsWithBlobs && *importantBuff == 1))
					(*dst)[0] = palette_transparency_lookup[(*dst)[0]][(*src)[0]]; //Fluffy: Transparency
					
				if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
					importantBuff++;
			}
		} else {
			for (i = 0; i < n; i++, (*src)++, (*dst)++, mask <<= 1) {
				if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
					(*dst)[0] = palette_transparency_lookup[tbl[(*src)[0]]][*importantBuff]; //Fluffy: Draw silhoutte using colour in important buffer
				else if (mask & 0x80000000 || ((options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) && *importantBuff == 0))
					(*dst)[0] = tbl[(*src)[0]]; //Draw partially lit pixel
				else if (options_transparency || (options_opaqueWallsWithBlobs && *importantBuff == 1))
					(*dst)[0] = palette_transparency_lookup[(*dst)[0]][tbl[(*src)[0]]]; //Fluffy: Transparency
					
				if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
					importantBuff++;
			}
		}
	}
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
