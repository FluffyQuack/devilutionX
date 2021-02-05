/**
 * @file engine.cpp
 *
 * Implementation of basic engine helper functions:
 * - Sprite blitting
 * - Drawing
 * - Angle calculation
 * - RNG
 * - Memory allocation
 * - File loading
 * - Video playback
 */
#include "all.h"
#include "../3rdParty/Storm/Source/storm.h"
#include "Render/render-cel.h" //Fluffy: For truecolour rendering

DEVILUTION_BEGIN_NAMESPACE

/** automap pixel color 8-bit (palette entry) */
char gbPixelCol;
/** flip - if y < x */
BOOL gbRotateMap;
/** Seed value before the most recent call to SetRndSeed() */
int orgseed;
/** Width of sprite being blitted */
int sgnWidth;
/** Current game seed */
int sglGameSeed;
static CCritSect sgMemCrit;
/** Number of times the current seed has been fetched */
int SeedCount;
/** valid - if x/y are in bounds */
BOOL gbNotInView;

/**
 * Specifies the increment used in the Borland C/C++ pseudo-random.
 */
const int RndInc = 1;

/**
 * Specifies the multiplier used in the Borland C/C++ pseudo-random number generator algorithm.
 */
const int RndMult = 0x015A4E35;

/**
 * @brief Blit CEL sprite to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelDraw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	CelBlitFrame(&gpBuffer[sx + BUFFER_WIDTH * sy], pCelBuff, nCel, nWidth);
}

/**
 * @brief Blit a given CEL frame to the given buffer
 * @param pBuff Target buffer
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelBlitFrame(BYTE *pBuff, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes;

	assert(pCelBuff != NULL);
	assert(pBuff != NULL);

	pRLEBytes = CelGetFrame(pCelBuff, nCel, &nDataSize);
	CelBlitSafe(pBuff, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Same as CelDraw but with the option to skip parts of the top and bottom of the sprite
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDraw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes;
	int nDataSize;

	assert(gpBuffer);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	CelBlitSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth);
}

/**
 * @brief Blit CEL sprite, and apply lighting, to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelDrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, BYTE *tbl)
{
	int nDataSize;
	BYTE *pDecodeTo, *pRLEBytes;

	assert(gpBuffer);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrame(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light_table_index || tbl)
		CelBlitLightSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth, tbl);
	else
		CelBlitSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Same as CelDrawLight but with the option to skip parts of the top and bottom of the sprite
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes, *pDecodeTo;

	assert(gpBuffer);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light_table_index)
		CelBlitLightSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth, NULL);
	else
		CelBlitSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Blit CEL sprite, and apply lighting, to the back buffer at the given coordinates, translated to a red hue
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 * @param light Light shade to use
 */
void CelDrawLightRed(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, char light)
{
	int nDataSize, w, idx;
	BYTE *pRLEBytes, *dst, *tbl;

	assert(gpBuffer);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	idx = light4flag ? 1024 : 4096;
	if (light == 2)
		idx += 256; // gray colors
	if (light >= 4)
		idx += (light - 1) << 8;

	BYTE width;
	BYTE *end;

	tbl = &pLightTbl[idx];
	end = &pRLEBytes[nDataSize];

	for (; pRLEBytes != end; dst -= BUFFER_WIDTH + nWidth) {
		for (w = nWidth; w;) {
			width = *pRLEBytes++;
			if (!(width & 0x80)) {
				w -= width;
				while (width) {
					*dst = tbl[*pRLEBytes];
					pRLEBytes++;
					dst++;
					width--;
				}
			} else {
				width = -(char)width;
				dst += width;
				w -= width;
			}
		}
	}
}

/**
 * @brief Blit CEL sprite to the given buffer, checks for drawing outside the buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
void CelBlitSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth)
{
	int i, w;
	BYTE width;
	BYTE *src, *dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer);

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					memcpy(dst, src, width);
				}
				src += width;
				dst += width;
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}

/**
 * @brief Same as CelClippedDraw but checks for drawing outside the buffer
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawSafe(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	if (options_32bitRendering) {
		CelClippedDrawSafe_To32BitBuffer(sx, sy, pCelBuff, nCel, nWidth);
		return;
	}

	BYTE *pRLEBytes;
	int nDataSize;

	assert(gpBuffer);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	CelBlitSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth);
}

/**
 * @brief Blit CEL sprite, and apply lighting, to the given buffer, checks for drawing outside the buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 * @param tbl Palette translation table
 */
void CelBlitLightSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE *tbl)
{
	int i, w;
	BYTE width;
	BYTE *src, *dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer);

	src = pRLEBytes;
	dst = pDecodeTo;
	if (tbl == NULL)
		tbl = &pLightTbl[light_table_index * 256];
	w = nWidth;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					if (width & 1) {
						dst[0] = tbl[src[0]];
						src++;
						dst++;
					}
					width >>= 1;
					if (width & 1) {
						dst[0] = tbl[src[0]];
						dst[1] = tbl[src[1]];
						src += 2;
						dst += 2;
					}
					width >>= 1;
					for (; width; width--) {
						dst[0] = tbl[src[0]];
						dst[1] = tbl[src[1]];
						dst[2] = tbl[src[2]];
						dst[3] = tbl[src[3]];
						src += 4;
						dst += 4;
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}

//Fluffy: Same as CelBlitLightSafe but with proper transparency (not dithered)
void CelBlitLightSafe_RealTransparency(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE *tbl)
{
	int i, w;
	BYTE width;
	BYTE *src, *dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer);

	src = pRLEBytes;
	dst = pDecodeTo;
	if (tbl == NULL)
		tbl = &pLightTbl[light_table_index * 256];
	w = nWidth;

	BYTE *importantBuff; //Fluffy: We use this for wall transparency (if certain toggles are turned on)
	if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
		importantBuff = gpBuffer_important + (dst - gpBuffer);

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					if (width & 1) {
						if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
							dst[0] = palette_transparency_lookup[*importantBuff][tbl[src[0]]]; //Render silhoutte using colour saved in important buffer
						else if (!options_opaqueWallsWithSilhouette && (!options_opaqueWallsWithBlobs || *importantBuff == 1))
							dst[0] = palette_transparency_lookup[dst[0]][tbl[src[0]]]; //Transparency
						else
							dst[0] = tbl[src[0]]; //Opaque
						src++;
						dst++;
						if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
							importantBuff++;
					}
					width >>= 1;
					if (width & 1) {
						for (int j = 0; j < 2; j++) {
							if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
								dst[j] = palette_transparency_lookup[*importantBuff][tbl[src[j]]]; //Render silhoutte using colour saved in important buffer
							else if (!options_opaqueWallsWithSilhouette && (!options_opaqueWallsWithBlobs || *importantBuff == 1))
								dst[j] = palette_transparency_lookup[dst[j]][tbl[src[j]]]; //Transparency
							else
								dst[j] = tbl[src[j]]; //Opaque
						}
						src += 2;
						dst += 2;
						if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
							importantBuff += 2;
					}
					width >>= 1;
					for (; width; width--) {
						for (int j = 0; j < 4; j++) {
							if (options_opaqueWallsWithSilhouette && *importantBuff != 0)
								dst[j] = palette_transparency_lookup[*importantBuff][tbl[src[j]]]; //Render silhoutte using colour saved in important buffer
							else if (!options_opaqueWallsWithSilhouette && (!options_opaqueWallsWithBlobs || *importantBuff == 1))
								dst[j] = palette_transparency_lookup[dst[j]][tbl[src[j]]]; //Transparency
							else
								dst[j] = tbl[src[j]]; //Opaque
						}
						src += 4;
						dst += 4;
						if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
							importantBuff += 4;
					}
				} else {
					src += width;
					dst += width;
					if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
						importantBuff += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
				if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
					importantBuff += width;
			}
		}
		if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
			importantBuff -= BUFFER_WIDTH + w;
	}
}

/**
 * @brief Same as CelBlitLightSafe, with transparancy applied
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
void CelBlitLightTransSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth)
{
	int w;
	BOOL shift;
	BYTE *tbl;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer);

	int i;
	BYTE width;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	tbl = &pLightTbl[light_table_index * 256];
	w = nWidth;
	shift = (BYTE)(size_t)dst & 1;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w, shift = (shift + 1) & 1) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					if (((BYTE)(size_t)dst & 1) == shift) {
						if (!(width & 1)) {
							goto L_ODD;
						} else {
							src++;
							dst++;
						L_EVEN:
							width >>= 1;
							if (width & 1) {
								dst[0] = tbl[src[0]];
								src += 2;
								dst += 2;
							}
							width >>= 1;
							for (; width; width--) {
								dst[0] = tbl[src[0]];
								dst[2] = tbl[src[2]];
								src += 4;
								dst += 4;
							}
						}
					} else {
						if (!(width & 1)) {
							goto L_EVEN;
						} else {
							dst[0] = tbl[src[0]];
							src++;
							dst++;
						L_ODD:
							width >>= 1;
							if (width & 1) {
								dst[1] = tbl[src[1]];
								src += 2;
								dst += 2;
							}
							width >>= 1;
							for (; width; width--) {
								dst[1] = tbl[src[1]];
								dst[3] = tbl[src[3]];
								src += 4;
								dst += 4;
							}
						}
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}

/**
 * @brief Same as CelBlitLightTransSafe
 * @param pBuff Target buffer
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedBlitLightTrans(BYTE *pBuff, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes;

	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	if (cel_transparency_active) {
		if (options_transparency)
			CelBlitLightSafe_RealTransparency(pBuff, pRLEBytes, nDataSize, nWidth, NULL); //Fluffy: Variant of below which renders proper transparency
		else
			CelBlitLightTransSafe(pBuff, pRLEBytes, nDataSize, nWidth);
	}
	else if (light_table_index)
		CelBlitLightSafe(pBuff, pRLEBytes, nDataSize, nWidth, NULL);
	else
		CelBlitSafe(pBuff, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Same as CelDrawLightRed but checks for drawing outside the buffer
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of cel
 * @param light Light shade to use
 */
void CelDrawLightRedSafe(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, char light)
{
	int nDataSize, w, idx;
	BYTE *pRLEBytes, *dst, *tbl;

	assert(gpBuffer);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	idx = light4flag ? 1024 : 4096;
	if (light == 2)
		idx += 256; // gray colors
	if (light >= 4)
		idx += (light - 1) << 8;

	tbl = &pLightTbl[idx];

	BYTE width;
	BYTE *end;

	end = &pRLEBytes[nDataSize];

	for (; pRLEBytes != end; dst -= BUFFER_WIDTH + nWidth) {
		for (w = nWidth; w;) {
			width = *pRLEBytes++;
			if (!(width & 0x80)) {
				w -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					while (width) {
						*dst = tbl[*pRLEBytes];
						pRLEBytes++;
						dst++;
						width--;
					}
				} else {
					pRLEBytes += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				w -= width;
			}
		}
	}
}

/**
 * @brief Blit to a buffer at given coordinates
 * @param pBuff Target buffer
 * @param x Cordinate in pBuff buffer
 * @param y Cordinate in pBuff buffer
 * @param wdt Width of pBuff
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of cel
 */
void CelBlitWidth(BYTE *pBuff, int x, int y, int wdt, BYTE *pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes, *dst, *end;

	assert(pCelBuff != NULL);
	assert(pBuff != NULL);

	int i, nDataSize;
	BYTE width;

	pRLEBytes = CelGetFrame(pCelBuff, nCel, &nDataSize);
	end = &pRLEBytes[nDataSize];
	dst = &pBuff[y * wdt + x];

	for (; pRLEBytes != end; dst -= wdt + nWidth) {
		for (i = nWidth; i;) {
			width = *pRLEBytes++;
			if (!(width & 0x80)) {
				i -= width;
				memcpy(dst, pRLEBytes, width);
				dst += width;
				pRLEBytes += width;
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}

//Fluffy: Just a quick debug function for highlighting pixels with the value 0 in the CEL format
/*
void CelBlit_ShadowPixels(char col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize, i, w;
	BYTE width;
	BYTE *src, *dst, *end;

	assert(pCelBuff != NULL);
	assert(gpBuffer);

	src = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];
	end = &src[nDataSize];
	w = nWidth;

	for (; src != end; dst -= BUFFER_WIDTH + w) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					for (int j = 0; j < width; j++) {
						if (*src == 0)
							*dst = col;
						src++;
						dst++;
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}
*/

inline static void DrawImportantAsEllipse_DrawHorizontalLine(BYTE *dst, BYTE *buffStart, BYTE *buffEnd, int length)
{
	if (dst < buffStart) {
		length -= buffStart - dst;
		dst = buffStart;
	}
	if (length > 0) {
		if (&dst[length] > buffEnd)
			length -= &dst[length] - buffEnd;
		if (length > 0)
			memset(dst, 1, length);
	}
}

static void DrawImportantAsEllipse(int sx, int sy, int height, int width, double scale)
{
	//Draw ellipse
	BYTE *dst = &gpBuffer_important[sx + BUFFER_WIDTH * sy] + -(BUFFER_WIDTH * (height / 2)) + (width / 2); //Calculate middle position of sprite which is our origin position for the ellipse
	BYTE *buffEnd = gpBuffer_important + (gpBufEnd - gpBuffer);
	BYTE *buffStart = gpBuffer_important + (gpBufStart - gpBuffer);

	//Adjust scale
	height /= scale;
	width /= scale;

	//Draw the middle row
	BYTE *dstBeg = &dst[-(width - 1)]; //To make the ellipse smoother, I reduce the length by one (otherwise the middle would stick out by one pixel)
	int length = (width * 2) - 1;
	DrawImportantAsEllipse_DrawHorizontalLine(dstBeg, buffStart, buffEnd, length);

	//Now we move down the ellipse and render rows (we also invert Y to render the upper rows)
	int hh = height * height;
	int ww = width * width;
	int hhww = hh * ww;
	int curWidth = width; //Width of the current row
	int widthReduce = 0; //How much width is reduced with current iteration
	for (int y = 1; y <= height; y++) { //Iterate through rows (starting from one row ahead of the middle)
		int x = curWidth - (widthReduce - 1);
		while (x > 0) { //Determine length of this row
			if (x * x * hh + y * y * ww <= hhww)
				break;
			x--;
		}
		widthReduce = curWidth - x;
		curWidth = x;

		//Lower row
		dstBeg = &dst[(y * BUFFER_WIDTH) - x];
		length = (x * 2) + 1;
		if (length == 1) //Avoid having the final part of the ellipse be a single pixel
			continue;
		DrawImportantAsEllipse_DrawHorizontalLine(dstBeg, buffStart, buffEnd, length);

		//Upper row
		dstBeg = &dst[(-y * BUFFER_WIDTH) - x];
		length = (x * 2) + 1;
		DrawImportantAsEllipse_DrawHorizontalLine(dstBeg, buffStart, buffEnd, length);
	}
}

//Fluffy: Draw an ellipse shape to the important buffer
void CelDrawToImportant_Ellipse(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize, w;
	BYTE *src, *dst, *end;
	BYTE width;

	assert(pCelBuff != NULL);
	assert(gpBuffer);

	src = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	end = &src[nDataSize];
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	//Get height of sprite
	int height = 0;
	BYTE *srcBack = src;
	while (src != end) {
		height++;
		for (int i = nWidth; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				src += width;
			} else {
				width = -(char)width;
				i -= width;
			}
		}
	}

	DrawImportantAsEllipse(sx, sy, height, nWidth, 2.5);
}

//Fluffy: Draw a cel as solid colour (with or without outline) to the "important buffer" (used for wall rendering)
void CelDrawToImportant(char col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, BOOL outline)
{
	int nDataSize, w;
	BYTE *src, *end;
	BYTE width;

	assert(pCelBuff != NULL);
	assert(gpBuffer_important);

	src = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	end = &src[nDataSize];
	BYTE *dst = &gpBuffer_important[sx + BUFFER_WIDTH * sy];
	BYTE *buffEnd = gpBuffer_important + (gpBufEnd - gpBuffer);
	BYTE *buffStart = gpBuffer_important + (gpBufStart - gpBuffer);

	for (; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (w = nWidth; w;) {
			width = *src++;
			if (!(width & 0x80)) {
				w -= width;
				if (dst < buffEnd && dst > buffStart) {
					if (dst >= buffEnd - BUFFER_WIDTH) {
						while (width) {
							if (*src++) {
								if (outline) {
									dst[-BUFFER_WIDTH] = col;
									dst[-1] = col;
									dst[1] = col;
								} else
									dst[0] = col;
							}
							dst++;
							width--;
						}
					} else {
						while (width) {
							if (*src++) {
								if (outline) {
									dst[-BUFFER_WIDTH] = col;
									dst[-1] = col;
									dst[1] = col;
									dst[BUFFER_WIDTH] = col;
								} else
									dst[0] = col;
							}
							dst++;
							width--;
						}
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				w -= width;
			}
		}
	}
}

//Fluffy: Same as CelBlitOutline() but it only draws pixels for the outline and nothing else
void CelBlitOutline_Precise(char col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
/*
	TODO: This function doesn't work well because 0 isn't only used for shadow, it's used for a lot of different pixels. I think the only proper fix would be to alter the assets

	One possible fix (though it would need a lot of work) is to do a "floodfill" action on assets for pixels of value 0, and then check if those resulting regions touch empty space. If they do, we assume it's a shadow and we separate them
	That would work for most sprites, but there are sprites which would need manual attention. For instance, the shields for skeletons are so dark it uses value 0 along its edge
*/
	int nDataSize, w;
	BYTE *src, *dst, *end;
	BYTE width;

	assert(pCelBuff != NULL);
	assert(gpBuffer);

	src = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	end = &src[nDataSize];
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	//Get height of sprite
	int height = 0;
	BYTE *srcBack = src;
	while (src != end) {
		height++;
		for (int i = nWidth; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				src += width;
			} else {
				width = -(char)width;
				i -= width;
			}
		}
	}

	//Write sprite to buffer (we only care about what pixels are opaque or not, so we don't add the real colour values)
	BYTE *buffer = new BYTE[nWidth * height];
	BYTE *bufferPtr = buffer;
	src = srcBack;
	while (src != end) {
		for (int i = nWidth; i;) {
			width = *src++;
			if (!(width & 0x80)) { //Run-length encoding. Positive signed byte means it defines quantity of bytes with image data
				for (int j = 0; j < width; j++) { //We have to go pixel by pixel here because we want to skip shadow pixels
					if (*src != 0)
						*bufferPtr = 1;
					else
						*bufferPtr = 0;
					src++;
					bufferPtr += 1;
				}
			} else { //Negative signed byte means it's defining quantity of skipped pixels
				width = -(char)width;
				memset(bufferPtr, 0, width);
				bufferPtr += width;
			}
			i -= width;
		}
	}
	assert(buffer + (nWidth * height) == bufferPtr);

	//Draw outline
	bufferPtr = buffer;
	BYTE *bufferEnd = buffer + (nWidth * height);
	BYTE *bufferOneRow = buffer + nWidth;
	while (bufferPtr < bufferEnd) {
		for (int i = nWidth; i;) {
			if (*bufferPtr == 1 && dst <= gpBufEnd && dst >= gpBufStart) {
				if (dst < gpBufEnd - BUFFER_WIDTH && (bufferPtr < bufferOneRow || bufferPtr[-nWidth] == 0))
					dst[BUFFER_WIDTH] = col;
				if (i == nWidth || bufferPtr <= buffer || bufferPtr[-1] == 0)
					dst[-1] = col;
				if (i == 1 || bufferPtr + 1 >= bufferEnd || bufferPtr[1] == 0)
					dst[1] = col;
				if (bufferPtr + nWidth >= bufferEnd || bufferPtr[nWidth] == 0)
					dst[-BUFFER_WIDTH] = col;
			}
			bufferPtr += 1;
			dst += 1;
			i--;
		}
		dst -= BUFFER_WIDTH + nWidth;
	}
	delete[] buffer;

	//TODO: We can greatly optimize this function by making it more similar to the CelBlitOutline() function but keeping the previous horizontal line as a buffer so we can compare against that as we process line by line
}

/**
 * @brief Blit a solid colder shape one pixel larger then the given sprite shape, to the back buffer at the given coordianates
 * @param col Color index from current palette
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelBlitOutline(char col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize, w;
	BYTE *src, *dst, *end;
	BYTE width;

	assert(pCelBuff != NULL);
	assert(gpBuffer);

	src = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	end = &src[nDataSize];
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	for (; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (w = nWidth; w;) {
			width = *src++;
			if (!(width & 0x80)) {
				w -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					if (dst >= gpBufEnd - BUFFER_WIDTH) {
						while (width) {
							if (*src++) {
								dst[-BUFFER_WIDTH] = col;
								dst[-1] = col;
								dst[1] = col;
							}
							dst++;
							width--;
						}
					} else {
						while (width) {
							if (*src++) {
								dst[-BUFFER_WIDTH] = col;
								dst[-1] = col;
								dst[1] = col;
								dst[BUFFER_WIDTH] = col;
							}
							dst++;
							width--;
						}
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				w -= width;
			}
		}
	}
}

/**
 * @brief Set the value of a single pixel in the back buffer, checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param col Color index from current palette
 */
void ENG_set_pixel(int sx, int sy, BYTE col)
{
	BYTE *dst;

	assert(gpBuffer);

	if (sy < 0 || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X)
		return;

	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (dst < gpBufEnd && dst > gpBufStart)
		*dst = col;
}

/**
 * @brief Set the value of a single pixel in the back buffer to that of gbPixelCol, checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void engine_draw_pixel(int sx, int sy)
{
	BYTE *dst;

	assert(gpBuffer);

	if (gbRotateMap) {
		if (gbNotInView && (sx < 0 || sx >= SCREEN_HEIGHT + SCREEN_Y || sy < SCREEN_X || sy >= SCREEN_WIDTH + SCREEN_X))
			return;
		dst = &gpBuffer[sy + BUFFER_WIDTH * sx];
	} else {
		if (gbNotInView && (sy < 0 || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X))
			return;
		dst = &gpBuffer[sx + BUFFER_WIDTH * sy];
	}

	if (dst < gpBufEnd && dst > gpBufStart)
		*dst = gbPixelCol;
}

/**
 * @brief Draw a line on the back buffer
 * @param x0 Back buffer coordinate
 * @param y0 Back buffer coordinate
 * @param x1 Back buffer coordinate
 * @param y1 Back buffer coordinate
 * @param col Color index from current palette
 */
void DrawLine(int x0, int y0, int x1, int y1, BYTE col)
{
	int i, dx, dy, steps;
	float ix, iy, sx, sy;

	dx = x1 - x0;
	dy = y1 - y0;
	steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
	ix = dx / (float)steps;
	iy = dy / (float)steps;
	sx = x0;
	sy = y0;

	for (i = 0; i <= steps; i++, sx += ix, sy += iy) {
		ENG_set_pixel(sx, sy, col);
	}
}

/**
 * @brief Calculate the best fit direction between two points
 * @param x1 Tile coordinate
 * @param y1 Tile coordinate
 * @param x2 Tile coordinate
 * @param y2 Tile coordinate
 * @return A value from the direction enum
 */
int GetDirection(int x1, int y1, int x2, int y2)
{
	int mx, my;
	int md, ny;

	mx = x2 - x1;
	my = y2 - y1;

	if (mx >= 0) {
		if (my >= 0) {
			md = DIR_S;
			if (2 * mx < my)
				md = DIR_SW;
		} else {
			my = -my;
			md = DIR_E;
			if (2 * mx < my)
				md = DIR_NE;
		}
		if (2 * my < mx)
			return DIR_SE;
	} else {
		if (my >= 0) {
			ny = -mx;
			md = DIR_W;
			if (2 * ny < my)
				md = DIR_SW;
		} else {
			ny = -mx;
			my = -my;
			md = DIR_N;
			if (2 * ny < my)
				md = DIR_NE;
		}
		if (2 * my < ny)
			return DIR_NW;
	}

	return md;
}

/**
 * @brief Set the RNG seed
 * @param s RNG seed
 */
void SetRndSeed(int s)
{
	SeedCount = 0;
	sglGameSeed = s;
	orgseed = s;
}

/**
 * @brief Advance the internal RNG seed and return the new value
 * @return RNG seed
 */
int AdvanceRndSeed()
{
	SeedCount++;
	sglGameSeed = static_cast<unsigned int>(RndMult) * sglGameSeed + RndInc;
	return abs(sglGameSeed);
}

/**
 * @brief Get the current RNG seed
 * @return RNG seed
 */
int GetRndSeed()
{
	return abs(sglGameSeed);
}

/**
 * @brief Main RNG function
 * @param idx Unused
 * @param v The upper limit for the return value
 * @return A random number from 0 to (v-1)
 */
int random_(BYTE idx, int v)
{
	if (v <= 0)
		return 0;
	if (v < 0xFFFF)
		return (AdvanceRndSeed() >> 16) % v;
	return AdvanceRndSeed() % v;
}

/**
 * @brief Multithreaded safe malloc
 * @param dwBytes Byte size to allocate
 */
BYTE *DiabloAllocPtr(DWORD dwBytes)
{
	BYTE *buf;

	sgMemCrit.Enter();
	buf = (BYTE *)SMemAlloc(dwBytes, __FILE__, __LINE__, 0);
	sgMemCrit.Leave();

	if (buf == NULL) {
		char *text = "System memory exhausted.\n"
		             "Make sure you have at least 64MB of free system memory before running the game";
		ERR_DLG("Out of Memory Error", text);
	}

	return buf;
}

/**
 * @brief Multithreaded safe memfree
 * @param p Memory pointer to free
 */
void mem_free_dbg(void *p)
{
	if (p) {
		sgMemCrit.Enter();
		SMemFree(p, __FILE__, __LINE__, 0);
		sgMemCrit.Leave();
	}
}

/**
 * @brief Load a file in to a buffer
 * @param pszName Path of file
 * @param pdwFileLen Will be set to file size if non-NULL
 * @return Buffer with content of file
 */
BYTE *LoadFileInMem(const char *pszName, DWORD *pdwFileLen)
{
	HANDLE file;
	BYTE *buf;
	int fileLen;

	SFileOpenFile(pszName, &file);
	fileLen = SFileGetFileSize(file, NULL);

	if (pdwFileLen)
		*pdwFileLen = fileLen;

	if (!fileLen)
		app_fatal("Zero length SFILE:\n%s", pszName);

	buf = (BYTE *)DiabloAllocPtr(fileLen);

	SFileReadFile(file, buf, fileLen, NULL, NULL);
	SFileCloseFile(file);

	return buf;
}

/**
 * @brief Load a file in to the given buffer
 * @param pszName Path of file
 * @param p Target buffer
 * @return Size of file
 */
DWORD LoadFileWithMem(const char *pszName, BYTE *p)
{
	DWORD dwFileLen;
	HANDLE hsFile;

	assert(pszName);
	if (p == NULL) {
		app_fatal("LoadFileWithMem(NULL):\n%s", pszName);
	}

	SFileOpenFile(pszName, &hsFile);

	dwFileLen = SFileGetFileSize(hsFile, NULL);
	if (dwFileLen == 0) {
		app_fatal("Zero length SFILE:\n%s", pszName);
	}

	SFileReadFile(hsFile, p, dwFileLen, NULL, NULL);
	SFileCloseFile(hsFile);

	return dwFileLen;
}

/**
 * @brief Apply the color swaps to a CL2 sprite
 * @param p CL2 buffer
 * @param ttbl Palette translation table
 * @param nCel Frame number in CL2 file
 */
void Cl2ApplyTrans(BYTE *p, BYTE *ttbl, int nCel)
{
	int i, nDataSize;
	char width;
	BYTE *dst;

	assert(p != NULL);
	assert(ttbl != NULL);

	for (i = 1; i <= nCel; i++) {
		dst = CelGetFrame(p, i, &nDataSize) + 10;
		nDataSize -= 10;
		while (nDataSize) {
			width = *dst++;
			nDataSize--;
			assert(nDataSize >= 0);
			if (width < 0) {
				width = -width;
				if (width > 65) {
					nDataSize--;
					assert(nDataSize >= 0);
					*dst = ttbl[*dst];
					dst++;
				} else {
					nDataSize -= width;
					assert(nDataSize >= 0);
					while (width--) {
						*dst = ttbl[*dst];
						dst++;
					}
				}
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth Width of sprite
 */
static void Cl2BlitSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth)
{
	int w;
	char width;
	BYTE fill;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;

	while (nDataSize) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				fill = *src++;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width) {
						*dst = fill;
						dst++;
						width--;
					}
					if (!w) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				}
			} else {
				nDataSize -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width) {
						*dst = *src;
						src++;
						dst++;
						width--;
					}
					if (!w) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				} else {
					src += width;
				}
			}
		}
		while (width) {
			if (width > w) {
				dst += w;
				width -= w;
				w = 0;
			} else {
				dst += width;
				w -= width;
				width = 0;
			}
			if (!w) {
				w = nWidth;
				dst -= BUFFER_WIDTH + w;
			}
		}
	}
}

//Fluffy: Same as Cl2DrawToImportant() but we draw an ellipse based on center point of sprite
void Cl2DrawToImportant_Ellipse(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	gpBufEnd -= BUFFER_WIDTH;

	char width;
	BYTE *src = pRLEBytes;
	
	int w = nWidth;
	int rows = 0;
	while (nDataSize) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				src++;
			} else {
				nDataSize -= width;
				src += width;
			}
		}
		while (width) {
			if (width > w) {
				width -= w;
				w = 0;
			} else {
				w -= width;
				width = 0;
			}
			if (!w) {
				w = nWidth;
				rows++;
			}
		}
	}

	DrawImportantAsEllipse(sx, sy, rows, nWidth, 2.5);

	gpBufEnd += BUFFER_WIDTH;
}

//Fluffy: Similar to Cl2DrawOutline() but it writes to gpBuffer_important
void Cl2DrawToImportant(char col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, BOOL outline)
{
	int nDataSize;
	BYTE *pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	gpBufEnd -= BUFFER_WIDTH;

	char width;
	BYTE *src = pRLEBytes;
	BYTE *dst = &gpBuffer_important[sx + BUFFER_WIDTH * sy];
	BYTE *buffEnd = gpBuffer_important + (gpBufEnd - gpBuffer);
	BYTE *buffStart = gpBuffer_important + (gpBufStart - gpBuffer);
	int w = nWidth;

	int rows = 0;
	while (nDataSize) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				if (*src++ && dst < buffEnd && dst > buffStart) {
					w -= width;
					if (outline) {
						dst[-1] = col;
						dst[width] = col;
					}
					while (width) {
						if (outline) {
							dst[-BUFFER_WIDTH] = col;
							dst[BUFFER_WIDTH] = col;
							
						} else
							dst[0] = col;
						dst++;
						width--;
					}
					if (!w) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
						rows++;
					}
					continue;
				}
			} else {
				nDataSize -= width;
				if (dst < buffEnd && dst > buffStart) {
					w -= width;
					while (width) {
						if (*src++) {
							if (outline) {
								dst[-1] = col;
								dst[1] = col;
								dst[-BUFFER_WIDTH] = col;
								// BUGFIX: only set `if (dst+BUFFER_WIDTH < gpBufEnd)`
								dst[BUFFER_WIDTH] = col;
							} else
								dst[0] = col;
						}
						dst++;
						width--;
					}
					if (!w) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
						rows++;
					}
					continue;
				} else {
					src += width;
				}
			}
		}
		while (width) {
			if (width > w) {
				dst += w;
				width -= w;
				w = 0;
			} else {
				dst += width;
				w -= width;
				width = 0;
			}
			if (!w) {
				w = nWidth;
				dst -= BUFFER_WIDTH + w;
				rows++;
			}
		}
	}

	gpBufEnd += BUFFER_WIDTH;
}

/**
 * @brief Blit a solid colder shape one pixel larger then the given sprite shape, to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth Width of sprite
 * @param col Color index from current palette
 */
static void Cl2BlitOutlineSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, char col)
{
	int w;
	char width;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;

	while (nDataSize) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				if (*src++ && dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					dst[-1] = col;
					dst[width] = col;
					while (width) {
						dst[-BUFFER_WIDTH] = col;
						dst[BUFFER_WIDTH] = col;
						dst++;
						width--;
					}
					if (!w) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				}
			} else {
				nDataSize -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width) {
						if (*src++) {
							dst[-1] = col;
							dst[1] = col;
							dst[-BUFFER_WIDTH] = col;
							// BUGFIX: only set `if (dst+BUFFER_WIDTH < gpBufEnd)`
							dst[BUFFER_WIDTH] = col;
						}
						dst++;
						width--;
					}
					if (!w) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				} else {
					src += width;
				}
			}
		}
		while (width) {
			if (width > w) {
				dst += w;
				width -= w;
				w = 0;
			} else {
				dst += width;
				w -= width;
				width = 0;
			}
			if (!w) {
				w = nWidth;
				dst -= BUFFER_WIDTH + w;
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite, and apply lighting, to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth With of CL2 sprite
 * @param pTable Light color table
 */
static void Cl2BlitLightSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE *pTable)
{
	int w;
	char width;
	BYTE fill;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;
	sgnWidth = nWidth;

	while (nDataSize) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				fill = pTable[*src++];
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width) {
						*dst = fill;
						dst++;
						width--;
					}
					if (!w) {
						w = sgnWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				}
			} else {
				nDataSize -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width) {
						*dst = pTable[*src];
						src++;
						dst++;
						width--;
					}
					if (!w) {
						w = sgnWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				} else {
					src += width;
				}
			}
		}
		while (width) {
			if (width > w) {
				dst += w;
				width -= w;
				w = 0;
			} else {
				dst += width;
				w -= width;
				width = 0;
			}
			if (!w) {
				w = sgnWidth;
				dst -= BUFFER_WIDTH + w;
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite, to the back buffer at the given coordianates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2Draw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes;
	int nDataSize;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	Cl2BlitSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth);
}
/**
 * @brief Blit a solid colder shape one pixel larger then the given sprite shape, to the back buffer at the given coordianates
 * @param col Color index from current palette
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2DrawOutline(char col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	gpBufEnd -= BUFFER_WIDTH;
	Cl2BlitOutlineSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth,
	    col);
	gpBufEnd += BUFFER_WIDTH;
}

/**
 * @brief Blit CL2 sprite, and apply a given lighting, to the back buffer at the given coordianates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 * @param light Light shade to use
 */
void Cl2DrawLightTbl(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, char light)
{
	int nDataSize, idx;
	BYTE *pRLEBytes, *pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	idx = light4flag ? 1024 : 4096;
	if (light == 2)
		idx += 256; // gray colors
	if (light >= 4)
		idx += (light - 1) << 8;

	Cl2BlitLightSafe(
	    pDecodeTo,
	    pRLEBytes,
	    nDataSize,
	    nWidth,
	    &pLightTbl[idx]);
}

/**
 * @brief Blit CL2 sprite, and apply lighting, to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2DrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes, *pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light_table_index)
		Cl2BlitLightSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth, &pLightTbl[light_table_index * 256]);
	else
		Cl2BlitSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Fade to black and play a video
 * @param pszMovie file path of movie
 */
void PlayInGameMovie(const char *pszMovie)
{
	PaletteFadeOut(8);
	play_movie(pszMovie, FALSE);
	ClearScreenBuffer();
	force_redraw = 255;
	scrollrt_draw_game_screen(TRUE);
	PaletteFadeIn(8);
	force_redraw = 255;
}

DEVILUTION_END_NAMESPACE
